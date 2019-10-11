// video playback words
// from avlibrary
#include "video.h"
#include "graf.h"

#define MAX_QUEUE_SIZE (15 * 1024 * 1024)
#define SDL_AUDIO_BUFFER_SIZE 1024
#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000

typedef struct _PacketQueue {
	AVPacketList *first, *last;
	int nb_packets, size;
	SDL_mutex *cs;
	SDL_cond *cv;
} PacketQueue;

typedef struct _VideoState {
	AVFormatContext * pFormatCtx;
	AVCodecContext * audioCtx;
	AVCodecContext * videoCtx;
	struct SwrContext * pSwrCtx;
	struct SwsContext * pSwsCtx;
	int videoStream, audioStream;
	AVStream * audioSt;
	PacketQueue audioq;
	uint8_t audioBuf[(AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2];
	unsigned int audioBufSize, audioBufIndex;
	AVPacket audioPkt;
	AVPacket videoPkt;
	int hasAudioFrames;
	AVFrame * pAudioFrame;
	uint8_t audioConvertedData[(AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2];
	AVFrame * pFrameRGB;
	uint8_t * pFrameBuffer;
	AVStream * videoSt;
	PacketQueue videoq;
	int quit;
} VideoState;

SDL_Thread *hParseThread;
SDL_Thread *hVideoThread;

VideoState is;
int videow,videoh,videostride;

int sleepfps;
int videoa=0;

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
int rv;
if (!pq) return -1;
rv = av_packet_ref(&pkt, srcPkt);if (rv) return rv;
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

int PacketQueueGet(PacketQueue *pq, AVPacket *pkt, int block)
{
AVPacketList *elt;
int rv;
if (!pq || !pkt) return -1;
SDL_LockMutex(pq->cs);		
for (;;) {
	if (is.quit) { rv=-1;break; }
	elt=pq->first;
	if (elt) {
		pq->first = elt->next;
		if (!pq->first) pq->last = NULL;
		pq->nb_packets--;
		pq->size -= elt->pkt.size;
		*pkt = elt->pkt;
		av_free(elt);
		rv = 1;
		break; } 
	else if (!block) { rv=0;break; } 
	else { SDL_CondWait(pq->cv,pq->cs); }
	}
SDL_UnlockMutex(pq->cs);
return rv;
}

void PacketQueueFree(PacketQueue *pq)
{
if (!pq) return;
SDL_DestroyMutex(pq->cs);
SDL_DestroyCond(pq->cv);
AVPacketList *nn,*elt=pq->first;
while (elt) { nn=elt->next;av_free(elt);elt=nn; } 
}

////////////////////////////////////////////////////////////////
int DecodeAudioFrame(void)
{
int len2,dataSize=0,outSize=0,rv,hasPacket=0;
int64_t len1;
uint8_t *converted=&is.audioConvertedData[0];
for (;;) {
	while (is.hasAudioFrames) {
		rv = avcodec_receive_frame(is.audioCtx, is.pAudioFrame);
		if (rv) { is.hasAudioFrames = 0;break;	}
		dataSize = av_samples_get_buffer_size(NULL,is.audioCtx->channels,is.pAudioFrame->nb_samples,is.audioCtx->sample_fmt,1);
		outSize = av_samples_get_buffer_size(NULL,is.audioCtx->channels,is.pAudioFrame->nb_samples,AV_SAMPLE_FMT_FLT,1);
		len2 = swr_convert(is.pSwrCtx,&converted,is.pAudioFrame->nb_samples,(const uint8_t**)&is.pAudioFrame->data[0],is.pAudioFrame->nb_samples);
		memcpy(is.audioBuf, converted, outSize);
		dataSize = outSize;
		return dataSize;
		}
	if (hasPacket)	{ av_packet_unref(&is.audioPkt); }
	if (is.quit) return -1;
	if (PacketQueueGet(&is.audioq, &is.audioPkt, 1) < 0) return -1;
	hasPacket = 1;
	rv = avcodec_send_packet(is.audioCtx, &is.audioPkt);if (rv) return rv;
	is.hasAudioFrames = 1;
	}
return -1;
}

void AudioCallback(void *userdata, uint8_t * stream, int len)
{
int len1, audioSize;
while (len > 0) {
	if (is.audioBufIndex >= is.audioBufSize) {
		audioSize = DecodeAudioFrame();
		if (audioSize < 0) 	{ 
			is.audioBufSize = SDL_AUDIO_BUFFER_SIZE;
			memset(is.audioBuf, 0, sizeof(is.audioBuf));
		} else {
			is.audioBufSize = audioSize;
			}
		is.audioBufIndex = 0;
		}
	len1 = is.audioBufSize - is.audioBufIndex;
	if (len1 > len) len1 = len;
	memcpy(stream, (uint8_t *)is.audioBuf + is.audioBufIndex, len1);
	len -= len1;
	stream += len1;
	is.audioBufIndex += len1;
	}
}

int VideoThread(void* pUserData)
{
int rv,ms1;
AVFrame *pFrame = NULL;
pFrame = av_frame_alloc();
ms1=SDL_GetTicks()+sleepfps;
while (PacketQueueGet(&is.videoq, &is.videoPkt,1)>0) {
	rv = avcodec_send_packet(is.videoCtx, &is.videoPkt);if (rv<0) continue;
	while (!avcodec_receive_frame(is.videoCtx, pFrame)) {
		sws_scale(is.pSwsCtx,pFrame->data,pFrame->linesize,0,is.videoCtx->height,is.pFrameRGB->data,is.pFrameRGB->linesize);
		while (SDL_GetTicks()<ms1) ;
		ms1=SDL_GetTicks()+sleepfps;		
		if (is.quit) break;
		}
	if (is.quit) break;
	}
av_frame_free(&pFrame);
return 0;
}

int StreamComponentOpen(int streamIndex)
{
AVFormatContext *pFormatCtx=is.pFormatCtx;
AVCodecContext *codecCtx;
AVCodec *codec;
AVCodecParameters *codecPar;
SDL_AudioSpec wantedSpec = { 0 }, audioSpec = {0};
int rv, rgbFrameSize;
if (streamIndex < 0 || streamIndex >= pFormatCtx->nb_streams) return -1;
codecPar = pFormatCtx->streams[streamIndex]->codecpar;
codec = avcodec_find_decoder(codecPar->codec_id);if (!codec) return -1;
codecCtx = avcodec_alloc_context3(codec);if (!codecCtx) return -1;
rv=avcodec_parameters_to_context(codecCtx, codecPar);if (rv<0) { avcodec_free_context(&codecCtx);return rv;	}
rv=avcodec_open2(codecCtx, codec, NULL);if (rv<0) { avcodec_free_context(&codecCtx);return rv; }
if (codecPar->codec_type == AVMEDIA_TYPE_AUDIO) {
	is.audioCtx = codecCtx;
	is.audioStream = streamIndex;
	is.audioBufSize = 0;
	is.audioBufIndex = 0;
	is.audioSt = pFormatCtx->streams[streamIndex];
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
	if (SDL_OpenAudio(&wantedSpec, &audioSpec) < 0) { return -1; }
	PacketQueueInit(&is.audioq);
	SDL_PauseAudio(0);
} else if (codecPar->codec_type == AVMEDIA_TYPE_VIDEO) {
	is.videoCtx = codecCtx;
	is.videoStream = streamIndex;
	is.videoSt = pFormatCtx->streams[streamIndex];
	
	if (codecCtx->width>0) {
		videoh=codecCtx->height*videow/codecCtx->width;	// aspect ratio
		videostride=gr_ancho-videow;
	} else {
		videoh=1;videostride=1;
	}
	
	is.pSwsCtx = sws_getContext(codecCtx->width,codecCtx->height,codecCtx->pix_fmt,videow,videoh,AV_PIX_FMT_RGB32,SWS_BILINEAR,NULL,NULL,NULL);
	is.pFrameRGB=av_frame_alloc();
	rgbFrameSize=av_image_get_buffer_size(AV_PIX_FMT_RGB32,videow,videoh,8);
	is.pFrameBuffer=(uint8_t*)av_malloc(rgbFrameSize);
	rv=av_image_fill_arrays(&is.pFrameRGB->data[0],&is.pFrameRGB->linesize[0],is.pFrameBuffer,AV_PIX_FMT_RGB32,videow,videoh,1);

	PacketQueueInit(&is.videoq);
	hVideoThread = SDL_CreateThread(VideoThread,NULL, &is);
} else {
	avcodec_free_context(&codecCtx);
	return -1;
	}
	
return 0;
}

////////////////////////////////////////////////////////////////////////////
void videoresize(int vw)
{

videow=vw;	
videostride=gr_ancho-videow;
videoh=is.videoCtx->height*videow/is.videoCtx->width;	// aspect ratio
if (is.pFrameBuffer==NULL) return;
av_frame_free(&is.pFrameRGB);
av_free(is.pFrameBuffer);is.pFrameBuffer=NULL;
sws_freeContext(is.pSwsCtx);
is.pSwsCtx=sws_getContext(is.videoCtx->width,is.videoCtx->height,is.videoCtx->pix_fmt,
	videow,videoh,AV_PIX_FMT_RGB32,SWS_BILINEAR,NULL,NULL,NULL);
is.pFrameRGB=av_frame_alloc();
int rgbFrameSize=av_image_get_buffer_size(AV_PIX_FMT_RGB32,videow,videoh,8);
is.pFrameBuffer=(uint8_t*)av_malloc(rgbFrameSize);
av_image_fill_arrays(&is.pFrameRGB->data[0],&is.pFrameRGB->linesize[0],
	is.pFrameBuffer,AV_PIX_FMT_RGB32,videow,videoh,1);
}

int DecodeThread(void* pUserData)
{
AVPacket pkt;
int rv;
while(!is.quit) {
	if (is.videoq.size >= MAX_QUEUE_SIZE || is.audioq.size >= MAX_QUEUE_SIZE) { Sleep(10);continue; }
	rv = av_read_frame(is.pFormatCtx, &pkt);if (rv < 0) break;
	if (pkt.stream_index == is.audioStream) { PacketQueuePut(&is.audioq, &pkt); }
	else if (pkt.stream_index == is.videoStream) { PacketQueuePut(&is.videoq, &pkt); }
	av_packet_unref(&pkt);
	} 
while (!is.quit) { Sleep(100); }	
return 0;
}

////////////////////////////////////////////////////////////////////////////
void videoclose()
{
int tv;	
if (videoa==0) return;
SDL_CloseAudio();
is.quit=1;
if (hVideoThread) SDL_WaitThread(hVideoThread, &tv);
if (hParseThread) SDL_WaitThread(hParseThread, &tv);
PacketQueueFree(&is.videoq);
PacketQueueFree(&is.audioq);
if (is.pAudioFrame) av_frame_free(&is.pAudioFrame);
if (is.pFrameRGB) av_frame_free(&is.pFrameRGB);
if (is.pFrameBuffer) av_free(is.pFrameBuffer);
if (is.videoCtx) avcodec_free_context(&is.videoCtx);
if (is.audioCtx) avcodec_free_context(&is.audioCtx);
if (is.pSwrCtx) swr_free(&is.pSwrCtx);
if (is.pSwsCtx) sws_freeContext(is.pSwsCtx);
if (is.pFormatCtx)	avformat_close_input(&is.pFormatCtx);
avformat_network_deinit();
is.pFrameBuffer=NULL;
videoa=0;
}

////////////////////////////////////////////////////////////////////////////
void videoopen(char *filename,int vw)
{
if (videoa==1) videoclose();
videow=vw;videoh=1;
videostride=gr_ancho-videow;
avformat_network_init();
memset((void*)&is,0,sizeof(is));
int rv=0,audioStream=-1,videoStream=-1;
is.pFormatCtx = avformat_alloc_context();
rv=avformat_open_input(&is.pFormatCtx,filename,NULL,NULL);
rv=avformat_find_stream_info(is.pFormatCtx,NULL);
for (int s = 0; s < is.pFormatCtx->nb_streams; ++s) {
	if (is.pFormatCtx->streams[s]->codecpar->codec_type==AVMEDIA_TYPE_AUDIO && audioStream<0) { audioStream=s; } 
	else if (is.pFormatCtx->streams[s]->codecpar->codec_type==AVMEDIA_TYPE_VIDEO && videoStream<0) { videoStream=s; }
	}
is.audioStream = audioStream;
is.videoStream = videoStream;
if (audioStream >= 0) StreamComponentOpen(audioStream);
if (videoStream >= 0) StreamComponentOpen(videoStream);
AVRational fr = av_guess_frame_rate(is.pFormatCtx,is.videoSt, NULL);
sleepfps=1000/(int)((double)(fr.num+(fr.den/2))/fr.den);
hParseThread = SDL_CreateThread(DecodeThread,NULL,&is);
videoa=1;
return;
}

////////////////////////////////////////////////////////////////////////////
int redrawframe(int x,int y)
{
if (is.pFrameBuffer==NULL) return 0;
if (is.videoq.nb_packets==0 && is.audioq.nb_packets==0) {
	hVideoThread=hParseThread=0;
	videoclose();return -1; 
	}
int i,j;
Uint32 *s=(Uint32*)is.pFrameBuffer;
Uint32 *d=gr_buffer+(y*gr_ancho+x);
for (i=0;i<videoh;i++,d+=videostride) 
	for(j=0;j<videow;j++) *d++=*s++;
return 0;
}

