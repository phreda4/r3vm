// video playback words
// from avlibrary
#include "video.h"
#include "graf.h"

#define MAX_QUEUE_SIZE (15 * 1024 * 1024)
#define SDL_AUDIO_BUFFER_SIZE 1024
#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000

static SDL_AudioDeviceID audio_dev;

typedef struct _PacketQueue {
	AVPacketList *first, *last;
	int nb_packets, size;
	SDL_mutex *cs;
	SDL_cond *cv;
} PacketQueue;

typedef struct _VideoState {
	AVFormatContext *pFormatCtx;
	AVCodecContext *audioCtx;
	AVCodecContext *videoCtx;
	struct SwrContext *pSwrCtx;
	struct SwsContext *pSwsCtx;
	int videoStream, audioStream;
	AVStream *audioSt;
	PacketQueue audioq;
	unsigned int audioBufSize,audioBufIndex;
	AVPacket audioPkt;
	AVPacket videoPkt;
	int hasAudioFrames;
	AVFrame *pAudioFrame;
	AVFrame *pFrameRGB;
	uint8_t *pFrameBuffer;
	AVStream *videoSt;
	PacketQueue videoq;
	int quit;
	uint8_t audioBuf[(AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2];
	uint8_t audioConvertedData[(AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2];
} VideoState;

SDL_Thread *hParseThread;
SDL_Thread *hVideoThread;

VideoState is;
int videow,videoh,videostride;

int sleepfps;
int videoa=0;

int mem1;

////////////////////////////////////////////////////////////////
void PacketQueueInit(PacketQueue * pq)
{
memset(pq,0,sizeof(PacketQueue));
pq->cs=SDL_CreateMutex();
pq->cv=SDL_CreateCond();
}

int PacketQueuePut(PacketQueue * pq, const AVPacket * srcPkt)
{
AVPacketList *elt;
AVPacket pkt;
if (!pq) return -1;
if (av_packet_ref(&pkt, srcPkt)) return -1;
elt = (AVPacketList*)av_malloc(sizeof(AVPacketList));if (!elt) return -1;
elt->pkt = pkt;
elt->next = NULL;
SDL_LockMutex(pq->cs);
if (!pq->last) pq->first = elt; else pq->last->next = elt;
pq->last = elt;
pq->nb_packets++;
pq->size += elt->pkt.size;
SDL_CondSignal(pq->cv);
SDL_UnlockMutex(pq->cs);
return 0;
}

static int PacketQueueGet(PacketQueue *q, AVPacket *pkt, int block)
{
AVPacketList *pkt1;
int ret;
SDL_LockMutex(q->cs);
for (;;) {
	pkt1 = q->first;
	if (pkt1) {
		q->first = pkt1->next;
		if (!q->first)
			q->last=NULL;
            q->nb_packets--;
            q->size-=pkt1->pkt.size+sizeof(*pkt1);
            *pkt=pkt1->pkt;
//            printf("%d ",pkt1->pkt.size);
//            av_packet_unref(&pkt1->pkt);
			av_free(pkt1);
            ret=1;
            break;
        } else if (!block) {
            ret=0;
            break;
        } else {
            SDL_CondWait(q->cv, q->cs);
		}
    }
SDL_UnlockMutex(q->cs);
return ret;
}

static void packet_queue_flush(PacketQueue *q)
{
AVPacketList *pkt, *pkt1;
SDL_LockMutex(q->cs);
//int cc=0;
for (pkt=q->first;pkt;pkt=pkt1) {
    pkt1=pkt->next;
    //printf("%x ",pkt->pkt.size);
    av_packet_unref(&pkt->pkt);
    av_free(pkt);
//av_freep(&pkt);    
//    cc++;
    }
q->last=q->first=NULL;
q->nb_packets=q->size=0;
SDL_UnlockMutex(q->cs);
//printf("flush %d\n",cc);
}

static void PacketQueueFree(PacketQueue *q)
{
packet_queue_flush(q);
SDL_DestroyMutex(q->cs);
SDL_DestroyCond(q->cv);
}

////////////////////////////////////////////////////////////////
int DecodeAudioFrame(void)
{
int len2,dataSize=0,outSize=0,hasPacket=0;
int64_t len1;
uint8_t *converted=&is.audioConvertedData[0];
for (;;) {
	while (is.hasAudioFrames) {
		if (avcodec_receive_frame(is.audioCtx, is.pAudioFrame)) { is.hasAudioFrames = 0;break;	}
		dataSize = av_samples_get_buffer_size(NULL,is.audioCtx->channels,is.pAudioFrame->nb_samples,is.audioCtx->sample_fmt,1);
		outSize = av_samples_get_buffer_size(NULL,is.audioCtx->channels,is.pAudioFrame->nb_samples,AV_SAMPLE_FMT_FLT,1);
		len2 = swr_convert(is.pSwrCtx,&converted,is.pAudioFrame->nb_samples,(const uint8_t**)&is.pAudioFrame->data[0],is.pAudioFrame->nb_samples);
		memcpy(is.audioBuf, converted, outSize);
		dataSize = outSize;
		av_frame_unref(is.pAudioFrame);
		return dataSize;
		}
	if (hasPacket)	{ av_packet_unref(&is.audioPkt); }
	if (is.quit) return -1;
	if (PacketQueueGet(&is.audioq, &is.audioPkt, 1) < 0) return -1;
	hasPacket = 1;
	if (avcodec_send_packet(is.audioCtx, &is.audioPkt)) return -1;
	is.hasAudioFrames = 1;
	}
return -1;
}

void AudioCallback(void *userdata,uint8_t *stream, int len)
{
int len1, audioSize;
while (len>0) {
	if (is.audioBufIndex>=is.audioBufSize) {
		audioSize=DecodeAudioFrame();
		if (audioSize<0) { 
			is.audioBufSize=SDL_AUDIO_BUFFER_SIZE;
			memset(is.audioBuf,0,sizeof(is.audioBuf));
		} else {
			is.audioBufSize=audioSize;
			}
		is.audioBufIndex=0;
		}
	len1=is.audioBufSize-is.audioBufIndex;
	if (len1>len) len1=len;
	memcpy(stream,(uint8_t *)is.audioBuf+is.audioBufIndex,len1);
	len -= len1;
	stream += len1;
	is.audioBufIndex += len1;
	}
}

int VideoThread(void* pUserData)
{
AVFrame *pFrame=av_frame_alloc();
int ms1=SDL_GetTicks()+sleepfps;
while (!is.quit&&PacketQueueGet(&is.videoq, &is.videoPkt,1)>0) {
	if (avcodec_send_packet(is.videoCtx, &is.videoPkt)<0) continue;
	av_packet_unref(&is.videoPkt);
	if (!is.quit&&!avcodec_receive_frame(is.videoCtx, pFrame)) {
//		printf("%x %x %x|",is.pSwsCtx,is.pFrameRGB,is.pFrameBuffer);		
//		if (is.pSwsCtx && is.pFrameRGB && is.pFrameBuffer) {
			sws_scale(is.pSwsCtx,pFrame->data,pFrame->linesize,0,is.videoCtx->height,is.pFrameRGB->data,is.pFrameRGB->linesize);
			av_frame_unref(pFrame);
//			}
		}
	while (SDL_GetTicks()<ms1) Sleep(10);
	ms1=SDL_GetTicks()+sleepfps;		
	}
av_frame_free(&pFrame);
return 0;
}

////////////////////////////////////////////////////////////////////////////
void videoresize(int vw,int vh)
{
if (is.pFrameBuffer==NULL) return;
if (vw==videow) return;
videow=vw;	
videoh=is.videoCtx->height*vw/is.videoCtx->width;	// aspect ratio
if (videoh>vh) {
	videoh=vh;
	videow=is.videoCtx->width*vh/is.videoCtx->height;	// aspect ratio
	}
videostride=gr_ancho-vw;

av_free(is.pFrameBuffer);is.pFrameBuffer=NULL;
av_frame_unref(is.pFrameRGB);
av_frame_free(&is.pFrameRGB);
sws_freeContext(is.pSwsCtx);

is.pSwsCtx=sws_getContext(is.videoCtx->width,is.videoCtx->height,is.videoCtx->pix_fmt,videow,videoh,AV_PIX_FMT_RGB32,SWS_BILINEAR,NULL,NULL,NULL);
is.pFrameRGB=av_frame_alloc();
int rgbFrameSize=av_image_get_buffer_size(AV_PIX_FMT_RGB32,videow,videoh,8);
is.pFrameBuffer=(uint8_t*)av_malloc(rgbFrameSize);
av_image_fill_arrays(&is.pFrameRGB->data[0],&is.pFrameRGB->linesize[0],is.pFrameBuffer,AV_PIX_FMT_RGB32,videow,videoh,1);
}

int DecodeThread(void* pUserData)
{
AVPacket pkt;
while(!is.quit) {
	if (is.videoq.size>=MAX_QUEUE_SIZE || is.audioq.size>=MAX_QUEUE_SIZE) { Sleep(10);continue; }
	if (av_read_frame(is.pFormatCtx, &pkt)<0) break;
//	printf("(%d)",pkt.stream_index);
	if (pkt.stream_index == is.audioStream) 
		{ PacketQueuePut(&is.audioq, &pkt); }
	else if (pkt.stream_index == is.videoStream) 
		{ PacketQueuePut(&is.videoq, &pkt); }
	av_packet_unref(&pkt);
//	av_free_packet(&pkt);
	} 
while (!is.quit) { Sleep(100); }	
//av_free_packet(&pkt);
return 0;
}


////////////////////////////////////////////////////////////////////////////
void videoclose()
{
//printf("Close\n");
if (videoa==0) return;
is.quit=1;

SDL_WaitThread(hParseThread, NULL);
if (is.audioStream!=-1) {
//	printf("CloseA\n");	
	uint8_t data[1024] = {0};

    AVPacket *pktAudio=(AVPacket*)av_malloc(sizeof(AVPacket));
    av_init_packet(pktAudio);
    pktAudio->data = data;
    pktAudio->size = 1024;
    PacketQueuePut(&is.audioq, pktAudio);    
    SDL_CloseAudioDevice(audio_dev);    
    
	PacketQueueFree(&is.audioq);		
//	av_packet_unref(pktAudio);
//av_packet_free(is.audioPkt);
	swr_free(&is.pSwrCtx);	
	av_frame_unref(is.pAudioFrame);
	av_frame_free(&is.pAudioFrame);
	avcodec_free_context(&is.audioCtx);
	}
	
if (is.videoStream!=-1) {
//	printf("CloseV\n");		
	SDL_WaitThread(hVideoThread,NULL);
	PacketQueueFree(&is.videoq);	
	av_packet_unref(&is.videoPkt);
	//av_packet_free(&is.videoPkt);	
	
	av_free(is.pFrameBuffer);
	
	av_frame_unref(is.pFrameRGB);	
	av_frame_free(&is.pFrameRGB);
	sws_freeContext(is.pSwsCtx);
	avcodec_free_context(&is.videoCtx);
	}

avformat_close_input(&is.pFormatCtx);
//avformat_free_context(is.pFormatCtx);	
//avformat_network_deinit();
memset((void*)&is,0,sizeof(is));
videoa=0;
//printf("CloseEnd\n");
}

////////////////////////////////////////////////////////////////////////////
int StreamComponentAOpen(int streamIndex)
{
//if (streamIndex < 0 || streamIndex >= is.pFormatCtx->nb_streams) return -1;
	
SDL_AudioSpec wantedSpec={0},audioSpec={0};
int rv;

AVCodecContext *codecCtx;

AVCodecParameters *codecPar= is.pFormatCtx->streams[streamIndex]->codecpar;
AVCodec *codec=avcodec_find_decoder(codecPar->codec_id);
if (!codec) return -1;

codecCtx = avcodec_alloc_context3(codec);
if (!codecCtx) return -1;
rv=avcodec_parameters_to_context(codecCtx,codecPar);
if (rv<0) { avcodec_free_context(&codecCtx);return rv;	}
rv=avcodec_open2(codecCtx, codec, NULL);
if (rv<0) { avcodec_free_context(&codecCtx);return rv; }

is.audioCtx = codecCtx;
is.audioStream = streamIndex;
is.audioBufSize = 0;
is.audioBufIndex = 0;
is.audioSt = is.pFormatCtx->streams[streamIndex];
memset(&is.audioPkt, 0, sizeof(is.audioPkt));
is.pAudioFrame = av_frame_alloc();//if (!is.pAudioFrame) return -1;
is.pSwrCtx = swr_alloc();//if (!is.pSwrCtx) return -1;
av_opt_set_channel_layout(is.pSwrCtx, "in_channel_layout", codecCtx->channel_layout, 0);
av_opt_set_channel_layout(is.pSwrCtx, "out_channel_layout", codecCtx->channel_layout, 0);
av_opt_set_int(is.pSwrCtx, "in_sample_rate", codecCtx->sample_rate, 0);
av_opt_set_int(is.pSwrCtx, "out_sample_rate", codecCtx->sample_rate, 0);
av_opt_set_sample_fmt(is.pSwrCtx, "in_sample_fmt", codecCtx->sample_fmt, 0);
av_opt_set_sample_fmt(is.pSwrCtx, "out_sample_fmt", AV_SAMPLE_FMT_FLT, 0);
rv = swr_init(is.pSwrCtx);if (rv<0) return rv;
wantedSpec.channels = codecCtx->channels;
wantedSpec.freq = codecCtx->sample_rate;
wantedSpec.format = AUDIO_F32;
wantedSpec.silence = 0;
wantedSpec.samples = SDL_AUDIO_BUFFER_SIZE;
wantedSpec.callback = AudioCallback;
audio_dev=SDL_OpenAudioDevice(NULL,0,&wantedSpec,&audioSpec,0); //if (SDL_OpenAudio(&wantedSpec, &audioSpec) < 0) { return -1; }
PacketQueueInit(&is.audioq);
SDL_PauseAudioDevice(audio_dev, 0); //SDL_PauseAudio(0);
return 0;	
}

int StreamComponentVOpen(int streamIndex)
{
//if (streamIndex < 0 || streamIndex >= is.pFormatCtx->nb_streams) return -1;
int rv, rgbFrameSize;

AVCodecContext *codecCtx;
AVCodecParameters *codecPar= is.pFormatCtx->streams[streamIndex]->codecpar;
AVCodec *codec=avcodec_find_decoder(codecPar->codec_id);
if (!codec) return -1;
codecCtx = avcodec_alloc_context3(codec);
if (!codecCtx) return -1;
rv=avcodec_parameters_to_context(codecCtx, codecPar);
if (rv<0) { avcodec_free_context(&codecCtx);return rv;	}
rv=avcodec_open2(codecCtx, codec, NULL);
if (rv<0) { avcodec_free_context(&codecCtx);return rv; }

is.videoCtx = codecCtx;
is.videoStream = streamIndex;
is.videoSt = is.pFormatCtx->streams[streamIndex];

int vh=videoh,vw=videow;
videoh=codecCtx->height*vw/codecCtx->width;	// aspect ratio
if (videoh>vh) {
	videoh=vh;
	videow=codecCtx->width*vh/codecCtx->height;	// aspect ratio
	}
videostride=gr_ancho-videow;
	
	//printf("vw:%d vh:%d videow:%d videoh:%d stride:%d\n",vw,vh,videow,videoh,videostride);
is.pSwsCtx = sws_getContext(codecCtx->width,codecCtx->height,codecCtx->pix_fmt,videow,videoh,AV_PIX_FMT_RGB32,SWS_BILINEAR,NULL,NULL,NULL);
is.pFrameRGB=av_frame_alloc();
rgbFrameSize=av_image_get_buffer_size(AV_PIX_FMT_RGB32,videow,videoh,8);
is.pFrameBuffer=(uint8_t*)av_malloc(rgbFrameSize);
rv=av_image_fill_arrays(&is.pFrameRGB->data[0],&is.pFrameRGB->linesize[0],is.pFrameBuffer,AV_PIX_FMT_RGB32,videow,videoh,1);

PacketQueueInit(&is.videoq);
hVideoThread = SDL_CreateThread(VideoThread,NULL, &is);
return 0;
}

void videoopen(char *filename,int vw,int vh)
{
//printf("Open %s\n",filename);
if (videoa==1) { //printf("-");
	videoclose(); }

videow=vw;videoh=vh;
videostride=gr_ancho-videow;

//	av_register_all();
//	avformat_network_init();

memset((void*)&is,0,sizeof(is));
is.audioStream=is.videoStream=-1;
is.pFormatCtx=avformat_alloc_context();
if (avformat_open_input(&is.pFormatCtx,filename,NULL,NULL)<0) return;
if (avformat_find_stream_info(is.pFormatCtx,NULL)<0) return;
for (int s=0;s<is.pFormatCtx->nb_streams;++s) {
	if (is.pFormatCtx->streams[s]->codecpar->codec_type==AVMEDIA_TYPE_AUDIO && is.audioStream<0) 
		{ is.audioStream=s; } 
	else if (is.pFormatCtx->streams[s]->codecpar->codec_type==AVMEDIA_TYPE_VIDEO && is.videoStream<0) 
		{ is.videoStream=s; }
	}

if (is.audioStream>=0) {
	StreamComponentAOpen(is.audioStream);
	sleepfps=40;
	}
if (is.videoStream>=0) {
	StreamComponentVOpen(is.videoStream);
	AVRational fr=av_guess_frame_rate(is.pFormatCtx,is.videoSt, NULL);
	sleepfps=1000/(int)((double)(fr.num+(fr.den/2))/fr.den);
	}
hParseThread=SDL_CreateThread(DecodeThread,NULL,&is);

videoa=1;
//printf("OpenEnd\n");
return;
}

////////////////////////////////////////////////////////////////////////////
int redrawframe(int x,int y)
{
//printf("r%d %x-",videoa,is.videoq.nb_packets+is.audioq.nb_packets);
if (videoa==0) return -1;	
if (is.pFrameBuffer==NULL) return -1;
if (is.videoq.nb_packets+is.audioq.nb_packets==0) { //printf("*");
	videoclose();return -1; }
int i,j;
Uint32 *s=(Uint32*)is.pFrameBuffer;
Uint32 *d=gr_buffer+(y*gr_ancho+x);
for (i=0;i<videoh;i++,d+=videostride) 
	for(j=0;j<videow;j++) *d++=*s++;
	
return 0;
}

