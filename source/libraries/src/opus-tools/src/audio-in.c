/* Copyright 2000-2002, Michael Smith <msmith@xiph.org>
             2010, Monty <monty@xiph.org>
   AIFF/AIFC support from OggSquish, (c) 1994-1996 Monty <xiphmont@xiph.org>
   (From GPL code in oggenc relicensed by permission from Monty and Msmith)
   File: audio-in.c

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

   - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#if !defined(_LARGEFILE_SOURCE)
# define _LARGEFILE_SOURCE
#endif
#if !defined(_LARGEFILE64_SOURCE)
# define _LARGEFILE64_SOURCE
#endif
#if !defined(_FILE_OFFSET_BITS)
# define _FILE_OFFSET_BITS 64
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <math.h>

#include "stack_alloc.h"

#ifdef WIN32
# include <windows.h> /*GetFileType()*/
# include <io.h>      /*_get_osfhandle()*/
#endif

#ifdef ENABLE_NLS
#include <libintl.h>
#define _(X) gettext(X)
#else
#define _(X) (X)
#define textdomain(X)
#define bindtextdomain(X, Y)
#endif
#ifdef gettext_noop
#define N_(X) gettext_noop(X)
#else
#define N_(X) (X)
#endif

#include <ogg/ogg.h>
#include "opusenc.h"
#include "speex_resampler.h"
#include "lpc.h"
#include "opus_header.h"

/* Macros to read header data */
#define READ_U32_LE(buf) \
    (((buf)[3]<<24)|((buf)[2]<<16)|((buf)[1]<<8)|((buf)[0]&0xff))

#define READ_U16_LE(buf) \
    (((buf)[1]<<8)|((buf)[0]&0xff))

#define READ_U32_BE(buf) \
    (((buf)[0]<<24)|((buf)[1]<<16)|((buf)[2]<<8)|((buf)[3]&0xff))

#define READ_U16_BE(buf) \
    (((buf)[0]<<8)|((buf)[1]&0xff))

/* Define the supported formats here */
input_format formats[] = {
    {NULL, 0, NULL, NULL, NULL, NULL}
};

input_format *open_audio_file(FILE *in, oe_enc_opt *opt)
{
    int j=0;
    unsigned char *buf=NULL;
    int buf_size=0, buf_filled=0;
    int size,ret;

    while(formats[j].id_func)
    {
        size = formats[j].id_data_len;
        if(size >= buf_size)
        {
            buf = realloc(buf, size);
            buf_size = size;
        }

        if(size > buf_filled)
        {
            ret = fread(buf+buf_filled, 1, buf_size-buf_filled, in);
            buf_filled += ret;

            if(buf_filled < size)
            { /* File truncated */
                j++;
                continue;
            }
        }

        if(formats[j].id_func(buf, buf_filled))
        {
            /* ok, we now have something that can handle the file */
            if(formats[j].open_func(in, opt, buf, buf_filled)) {
                free(buf);
                return &formats[j];
            }
        }
        j++;
    }

    free(buf);

    return NULL;
}

typedef struct {
    audio_read_func real_reader;
    void *real_readdata;
    int channels;
    float scale_factor;
} scaler;

static long read_scaler(void *data, float *buffer, int samples) {
    scaler *d = data;
    long in_samples = d->real_reader(d->real_readdata, buffer, samples);
    int i;

    for(i=0; i < d->channels*in_samples; i++) {
       buffer[i] *= d->scale_factor;
    }

    return in_samples;
}

void setup_scaler(oe_enc_opt *opt, float scale) {
    scaler *d = calloc(1, sizeof(scaler));

    d->real_reader = opt->read_samples;
    d->real_readdata = opt->readdata;

    opt->read_samples = read_scaler;
    opt->readdata = d;
    d->channels = opt->channels;
    d->scale_factor = scale;
}

typedef struct {
    audio_read_func real_reader;
    void *real_readdata;
    ogg_int64_t *original_samples;
    int channels;
    int lpc_ptr;
    int *extra_samples;
    float *lpc_out;
} padder;

/* Read audio data, appending padding to make up any gap
 * between the available and requested number of samples
 * with LPC-predicted data to minimize the pertubation of
 * the valid data that falls in the same frame.
 */
static long read_padder(void *data, float *buffer, int samples) {
    padder *d = data;
    long in_samples = d->real_reader(d->real_readdata, buffer, samples);
    int i, extra=0;
    const int lpc_order=32;

    if(d->original_samples)*d->original_samples+=in_samples;

    if(in_samples<samples){
      if(d->lpc_ptr<0){
        d->lpc_out=calloc(d->channels * *d->extra_samples, sizeof(*d->lpc_out));
        if(in_samples>lpc_order*2){
          float *lpc=alloca(lpc_order*sizeof(*lpc));
          for(i=0;i<d->channels;i++){
            vorbis_lpc_from_data(buffer+i,lpc,in_samples,lpc_order,d->channels);
            vorbis_lpc_predict(lpc,buffer+i+(in_samples-lpc_order)*d->channels,
                               lpc_order,d->lpc_out+i,*d->extra_samples,d->channels);
          }
        }
        d->lpc_ptr=0;
      }
      extra=samples-in_samples;
      if(extra>*d->extra_samples)extra=*d->extra_samples;
      *d->extra_samples-=extra;
    }
    memcpy(buffer+in_samples*d->channels,d->lpc_out+d->lpc_ptr*d->channels,extra*d->channels*sizeof(*buffer));
    d->lpc_ptr+=extra;
    return in_samples+extra;
}

void setup_padder(oe_enc_opt *opt,ogg_int64_t *original_samples) {
    padder *d = calloc(1, sizeof(padder));

    d->real_reader = opt->read_samples;
    d->real_readdata = opt->readdata;

    opt->read_samples = read_padder;
    opt->readdata = d;
    d->channels = opt->channels;
    d->extra_samples = &opt->extraout;
    d->original_samples=original_samples;
    d->lpc_ptr = -1;
    d->lpc_out = NULL;
}

void clear_padder(oe_enc_opt *opt) {
    padder *d = opt->readdata;

    opt->read_samples = d->real_reader;
    opt->readdata = d->real_readdata;

    if(d->lpc_out)free(d->lpc_out);
    free(d);
}

typedef struct {
    SpeexResamplerState *resampler;
    audio_read_func real_reader;
    void *real_readdata;
    float *bufs;
    int channels;
    int bufpos;
    int bufsize;
    int done;
} resampler;

static long read_resampled(void *d, float *buffer, int samples)
{
    resampler *rs = d;
    int out_samples=0;
    float *pcmbuf;
    int *inbuf;
    pcmbuf=rs->bufs;
    inbuf=&rs->bufpos;
    while(out_samples<samples){
      int i;
      int reading, ret;
      unsigned in_len, out_len;
      out_len=samples-out_samples;
      reading=rs->bufsize-*inbuf;
      if(reading>1024)reading=1024;
      ret=rs->real_reader(rs->real_readdata, pcmbuf+*inbuf*rs->channels, reading);
      *inbuf+=ret;
      in_len=*inbuf;
      speex_resampler_process_interleaved_float(rs->resampler, pcmbuf, &in_len, buffer+out_samples*rs->channels, &out_len);
      out_samples+=out_len;
      if(ret==0&&in_len==0){
        for(i=out_samples*rs->channels;i<samples*rs->channels;i++)buffer[i]=0;
        return out_samples;
      }
      for(i=0;i<rs->channels*(*inbuf-(long int)in_len);i++)pcmbuf[i]=pcmbuf[i+rs->channels*in_len];
      *inbuf-=in_len;
    }
    return out_samples;
}

int setup_resample(oe_enc_opt *opt, int complexity, long outfreq) {
    resampler *rs = calloc(1, sizeof(resampler));
    int err;

    rs->bufsize = 5760*2; /* Have at least two output frames worth, just in case of ugly ratios */
    rs->bufpos = 0;

    rs->real_reader = opt->read_samples;
    rs->real_readdata = opt->readdata;
    rs->channels = opt->channels;
    rs->done = 0;
    rs->resampler = speex_resampler_init(rs->channels, opt->rate, outfreq, complexity, &err);
    if(err!=0)fprintf(stderr, _("resampler error: %s\n"), speex_resampler_strerror(err));

    opt->skip+=speex_resampler_get_output_latency(rs->resampler);

    rs->bufs = malloc(sizeof(float) * rs->bufsize * opt->channels);

    opt->read_samples = read_resampled;
    opt->readdata = rs;
    if(opt->total_samples_per_channel)
        opt->total_samples_per_channel = (int)((float)opt->total_samples_per_channel *
            ((float)outfreq/(float)opt->rate));
    opt->rate = outfreq;

    return 0;
}

void clear_resample(oe_enc_opt *opt) {
    resampler *rs = opt->readdata;

    opt->read_samples = rs->real_reader;
    opt->readdata = rs->real_readdata;
    speex_resampler_destroy(rs->resampler);

    free(rs->bufs);

    free(rs);
}

typedef struct {
    audio_read_func real_reader;
    void *real_readdata;
    float *bufs;
    float *matrix;
    int in_channels;
    int out_channels;
} downmix;

static long read_downmix(void *data, float *buffer, int samples)
{
    downmix *d = data;
    long in_samples = d->real_reader(d->real_readdata, d->bufs, samples);
    int i,j,k,in_ch,out_ch;

    in_ch=d->in_channels;
    out_ch=d->out_channels;

    for(i=0;i<in_samples;i++){
      for(j=0;j<out_ch;j++){
        float *samp;
        samp=&buffer[i*out_ch+j];
        *samp=0;
        for(k=0;k<in_ch;k++){
          *samp+=d->bufs[i*in_ch+k]*d->matrix[in_ch*j+k];
        }
      }
    }
    return in_samples;
}

int setup_downmix(oe_enc_opt *opt, int out_channels) {
    static const float stupid_matrix[7][8][2]={
      /*2*/  {{1,0},{0,1}},
      /*3*/  {{1,0},{0.7071f,0.7071f},{0,1}},
      /*4*/  {{1,0},{0,1},{0.866f,0.5f},{0.5f,0.866f}},
      /*5*/  {{1,0},{0.7071f,0.7071f},{0,1},{0.866f,0.5f},{0.5f,0.866f}},
      /*6*/  {{1,0},{0.7071f,0.7071f},{0,1},{0.866f,0.5f},{0.5f,0.866f},{0.7071f,0.7071f}},
      /*7*/  {{1,0},{0.7071f,0.7071f},{0,1},{0.866f,0.5f},{0.5f,0.866f},{0.6123f,0.6123f},{0.7071f,0.7071f}},
      /*8*/  {{1,0},{0.7071f,0.7071f},{0,1},{0.866f,0.5f},{0.5f,0.866f},{0.866f,0.5f},{0.5f,0.866f},{0.7071f,0.7071f}},
    };
    float sum;
    downmix *d;
    int i,j;

    if(opt->channels<=out_channels || out_channels>2 || opt->channels<=0 || out_channels<=0) {
        fprintf(stderr, _("Downmix must actually downmix and only knows mono/stereo out.\n"));
        return 0;
    }

    if(out_channels==2 && opt->channels>8) {
        fprintf(stderr, _("Downmix only knows how to mix >8ch to mono.\n"));
        return 0;
    }

    d = calloc(1, sizeof(downmix));
    d->bufs = malloc(sizeof(float)*opt->channels*4096);
    d->matrix = malloc(sizeof(float)*opt->channels*out_channels);
    d->real_reader = opt->read_samples;
    d->real_readdata = opt->readdata;
    d->in_channels=opt->channels;
    d->out_channels=out_channels;

    if(out_channels==1&&d->in_channels>8){
      for(i=0;i<d->in_channels;i++)d->matrix[i]=1.0f/d->in_channels;
    }else if(out_channels==2){
      for(j=0;j<d->out_channels;j++)
        for(i=0;i<d->in_channels;i++)d->matrix[d->in_channels*j+i]=
          stupid_matrix[opt->channels-2][i][j];
    }else{
      for(i=0;i<d->in_channels;i++)d->matrix[i]=
        (stupid_matrix[opt->channels-2][i][0])+
        (stupid_matrix[opt->channels-2][i][1]);
    }
    sum=0;
    for(i=0;i<d->in_channels*d->out_channels;i++)sum+=d->matrix[i];
    sum=(float)out_channels/sum;
    for(i=0;i<d->in_channels*d->out_channels;i++)d->matrix[i]*=sum;
    opt->read_samples = read_downmix;
    opt->readdata = d;

    opt->channels = out_channels;
    return out_channels;
}

void clear_downmix(oe_enc_opt *opt) {
    downmix *d = opt->readdata;

    opt->read_samples = d->real_reader;
    opt->readdata = d->real_readdata;
    opt->channels = d->in_channels; /* other things in cleanup rely on this */

    free(d->bufs);
    free(d->matrix);
    free(d);
}
