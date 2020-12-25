// Pablo Hugo Reda <pabloreda@gmail.com>
// rutinas de video

#ifndef VIDEO_H
#define VIDEO_H

#ifdef __cplusplus
extern "C"
{
#endif
#include <libavfilter/avfilter.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavutil/imgutils.h>
#include <libavutil/avstring.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
#ifdef __cplusplus
}
#endif

void videoopen(char *filename,int vw,int vh);
void videoclose();
void videoresize(int vw,int vh);
int redrawframe(int x,int y);
void initsoundffmpeg(void);
extern int mix_movie_channel;

#endif
