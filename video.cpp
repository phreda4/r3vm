// video playback words
// from avlibrary
#include "video.h"
#include "graf.h"

#define MAX_QUEUE_SIZE (15 * 1024 * 1024)
#define SDL_AUDIO_BUFFER_SIZE 4096
#define SDL_AUDIO_FREC 44100
#define SDL_FORMAT AV_SAMPLE_FMT_S16 //AV_SAMPLE_FMT_FLT

#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000

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
int padx,pady;

int sleepfps,sleepfr;
int videoa=0;

Uint8 chunk_buffer[SDL_AUDIO_BUFFER_SIZE];
void *is_audio;
int mix_movie_channel;
Mix_Chunk *chunk_movie;

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
elt = (AVPacketList*)av_malloc(sizeof(AVPacketList));
if (!elt) return -1;
elt->pkt = pkt;
elt->next = NULL;
SDL_LockMutex(pq->cs);
if (!pq->last) pq->first = elt; 
else pq->last->next = elt;
pq->last = elt;
pq->nb_packets++;
pq->size += elt->pkt.size;
SDL_CondSignal(pq->cv);
SDL_UnlockMutex(pq->cs);
return 0;
}

static int PacketQueueGet(PacketQueue *q, AVPacket *pkt)
{
AVPacketList *pkt1;
int ret=-1;
SDL_LockMutex(q->cs);
for (;;) {
	if (is.quit==1) { ret=-1;break; }
	pkt1 = q->first;
	if (pkt1) {
		q->first = pkt1->next;
		if (!q->first) q->last=NULL;
		q->nb_packets--;
		q->size-=pkt1->pkt.size+sizeof(*pkt1);
        *pkt=pkt1->pkt;
		av_free(pkt1);
        ret=1;break;
	} else { 
		ret=0;break; 
		}
    }
SDL_UnlockMutex(q->cs);
return ret;
}

static void packet_queue_flush(PacketQueue *q)
{
AVPacketList *pkt, *pkt1;
SDL_LockMutex(q->cs);
for (pkt=q->first;pkt;pkt=pkt1) {
    pkt1=pkt->next;
    av_packet_unref(&pkt->pkt);
    av_free(pkt);
//av_freep(&pkt);    
    }
q->last=q->first=NULL;
q->nb_packets=q->size=0;
SDL_UnlockMutex(q->cs);
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
uint8_t *converted=&is.audioConvertedData[0];
if (PacketQueueGet(&is.audioq, &is.audioPkt)<=0) return -1;
if (avcodec_send_packet(is.audioCtx, &is.audioPkt)) return -1;
av_packet_unref(&is.audioPkt);
if (avcodec_receive_frame(is.audioCtx, is.pAudioFrame)) return -1;
dataSize = av_samples_get_buffer_size(NULL,is.audioCtx->channels,is.pAudioFrame->nb_samples,is.audioCtx->sample_fmt,1);
outSize = av_samples_get_buffer_size(NULL,is.audioCtx->channels,is.pAudioFrame->nb_samples,SDL_FORMAT,1);
len2 = swr_convert(is.pSwrCtx,&converted,is.pAudioFrame->nb_samples,(const uint8_t**)&is.pAudioFrame->data[0],is.pAudioFrame->nb_samples);
memcpy(is.audioBuf, converted, outSize);
dataSize = outSize;
av_frame_unref(is.pAudioFrame);
return dataSize;
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

void mixer_effect_ffmpeg_cb(int chan,void *stream,int len,void *udata) { AudioCallback(udata, stream, len); }
void mixer_effectdone_ffmpeg_cb(int chan, void *udata) { }
	
int VideoThread(void* pUserData)
{
AVFrame *pFrame=av_frame_alloc();
int re,acc=sleepfr,ms1=SDL_GetTicks()+sleepfps;
while (!is.quit) { 
	re=PacketQueueGet(&is.videoq, &is.videoPkt);
	if (re==0) continue;
	if (re<0) break;
	if (avcodec_send_packet(is.videoCtx, &is.videoPkt)<0) continue;
	av_packet_unref(&is.videoPkt);
	if (!is.quit&&!avcodec_receive_frame(is.videoCtx, pFrame)) {
		sws_scale(is.pSwsCtx,pFrame->data,pFrame->linesize,0,is.videoCtx->height,is.pFrameRGB->data,is.pFrameRGB->linesize);
		av_frame_unref(pFrame);
		}
	while (SDL_GetTicks()<ms1) Sleep(10);
	ms1=SDL_GetTicks()+sleepfps-(SDL_GetTicks()-ms1);
	acc+=sleepfr;
	if (acc>1000) { acc-=1000;ms1++; }
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
padx=(vw-videow)/2;pady=(vh-videoh)/2;
av_free(is.pFrameBuffer);
is.pFrameBuffer=NULL;
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
	if (is.videoq.size>=MAX_QUEUE_SIZE || is.audioq.size>=MAX_QUEUE_SIZE) { Sleep(100);continue; }
	if (av_read_frame(is.pFormatCtx, &pkt)<0) break;
	if (pkt.stream_index == is.audioStream) 
		{ PacketQueuePut(&is.audioq, &pkt); }
	else if (pkt.stream_index == is.videoStream) 
		{ PacketQueuePut(&is.videoq, &pkt); }
	av_packet_unref(&pkt);
	} 
while (!is.quit) { Sleep(100); }	
return 0;
}


////////////////////////////////////////////////////////////////////////////
void videoclose()
{
if (videoa==0) return;
videoa=0;
is.quit=1;
Sleep(10);
SDL_WaitThread(hParseThread, NULL);
if (is.audioStream!=-1) {
//	Mix_Pause(mix_movie_channel);	
	Mix_HaltChannel(mix_movie_channel);	
	
	PacketQueueFree(&is.audioq);
	swr_free(&is.pSwrCtx);	
	av_frame_unref(is.pAudioFrame);
	av_frame_free(&is.pAudioFrame);
	avcodec_free_context(&is.audioCtx);
	}
	
if (is.videoStream!=-1) {
	SDL_WaitThread(hVideoThread,NULL);
	PacketQueueFree(&is.videoq);	
	av_freep(&is.videoPkt);	
	av_free(is.pFrameBuffer);//-&
	av_frame_unref(is.pFrameRGB);	
	av_frame_free(&is.pFrameRGB);
	sws_freeContext(is.pSwsCtx);
	avcodec_free_context(&is.videoCtx);
	}
avformat_close_input(&is.pFormatCtx);
//memset((void*)&is,0,sizeof(is));
}

////////////////////////////////////////////////////////////////////////////
int StreamComponentAOpen(int streamIndex)
{
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
av_opt_set_channel_layout(is.pSwrCtx, "in_channel_layout",codecCtx->channel_layout, 0);
av_opt_set_channel_layout(is.pSwrCtx, "out_channel_layout",codecCtx->channel_layout, 0);
av_opt_set_int(is.pSwrCtx, "in_sample_rate",codecCtx->sample_rate, 0);
av_opt_set_int(is.pSwrCtx, "out_sample_rate",SDL_AUDIO_FREC, 0);
av_opt_set_sample_fmt(is.pSwrCtx, "in_sample_fmt", codecCtx->sample_fmt, 0);
av_opt_set_sample_fmt(is.pSwrCtx, "out_sample_fmt", SDL_FORMAT, 0);
rv = swr_init(is.pSwrCtx);if (rv<0) return rv;
PacketQueueInit(&is.audioq);
//printf("r");
//Mix_Resume(mix_movie_channel);	

mix_movie_channel = Mix_PlayChannel(-1, chunk_movie, -1);
Mix_RegisterEffect(mix_movie_channel, mixer_effect_ffmpeg_cb, mixer_effectdone_ffmpeg_cb, &is_audio);

//printf("e\n");
return 0;	
}

int StreamComponentVOpen(int streamIndex)
{
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
padx=(vw-videow)/2;pady=(vh-videoh)/2;

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
if (videoa==1) { videoclose(); }

videow=vw;videoh=vh;
videostride=gr_ancho-videow;

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
	sleepfr=1000000/(int)((double)(fr.num+(fr.den/2))/fr.den)%1000;
	}
hParseThread=SDL_CreateThread(DecodeThread,NULL,&is);
videoa=1;
return;
}

////////////////////////////////////////////////////////////////////////////
int redrawframe(int x,int y)
{
if (videoa==0) return -1;	
if (is.pFrameBuffer==NULL) return -1;
if (is.videoq.nb_packets+is.audioq.nb_packets==0) return -1; 
int i,j;
Uint32 *s=(Uint32*)is.pFrameBuffer;
Uint32 *d=gr_buffer+((y+pady)*gr_ancho+(x+padx));
for(i=0;i<videoh;i++,d+=videostride) 
	for(j=0;j<videow;j++) *d++=*s++;
return 0;
}

void initsoundffmpeg(void)
{
//Mix_OpenAudio(SDL_AUDIO_FREC,AUDIO_F32,2,SDL_AUDIO_BUFFER_SIZE);
Mix_OpenAudio(SDL_AUDIO_FREC,AUDIO_S16SYS,2,SDL_AUDIO_BUFFER_SIZE);
memset(chunk_buffer, 0, sizeof(chunk_buffer));
chunk_movie = Mix_QuickLoad_RAW(chunk_buffer,SDL_AUDIO_BUFFER_SIZE);
/*
mix_movie_channel = Mix_PlayChannel(-1, chunk_movie, -1);
Mix_RegisterEffect(mix_movie_channel, mixer_effect_ffmpeg_cb, mixer_effectdone_ffmpeg_cb, &is_audio);
Mix_Pause(mix_movie_channel);
*/
SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");
}

