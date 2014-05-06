/*
   nlame - an alternative API for libmp3lame
   copyright (c) 2001-2014 Michael Fink
   Copyright (c) 2004 DeXT

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
  
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/
/*! \file nlame.h

   \brief nlame API header

   nlame is an alternative set of API functions for libmp3lame,
   the LAME mp3 encoding library. The goal of nlame is to provide a
   stable, small and easy to use API.

   API convention: functions look like: nlame_<topic>_<function>
   where topic is the general type and function is the functionality
   of that function.

   note that the API lacks some functionality, such as id3 stuff, ogg vorbis
   support or decoding. it's just a plain mp3 encoding lib.

   nLAME API version history:

   Version 0: initial API
   Version 1: introduced on 2005-11-30
      The following new variable values were added:
      nle_var_quality_value_high
      nle_var_quality_value_fast
      nle_var_find_replay_gain
      nle_var_decode_on_the_fly
      nle_var_framesize
    Version 2: introduced on 2006-03-01
      The nlame_encode_buffer_interleaved() function can now also handle the
      nle_buffer_int buffer type.

    Version 2: introduced on 2006-06-14
      nlame_encode_buffer_interleaved now supports the nle_buffer_int as
      type, too.

    Version 3: introduced on 2009-04-11
      Added nlame_id3tag_init() and nlame_id3tag_setfield_latin1()

    Version 4: introduced on 2009-10-26
      Added nlame_reinit_bitstream()

*/
/*! \defgroup nlame nlame Documentation

   documentation for the nlame API

*/
/*! \ingroup nlame */
/*@{ */

/* prevents multiple including */
#ifndef nlame_nlame_h_
#define nlame_nlame_h_

/* needed includes */
#include <stdio.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif


/* ------------------------------------------------------------- */


/* lame version info */

/*! type of version string that should be returned by nlame_lame_version_get */
typedef enum
{
   nle_lame_version_normal=0,
   nle_lame_version_short,
   nle_lame_version_psy
} nlame_lame_version_type;

/*! returns a lame version string */
const char* nlame_lame_version_get(nlame_lame_version_type type);


/* lame numerical version info */

/*! returns version number of libmp3lame */
void nlame_lame_version_get_num(int* major, int* minor, int* alpha, int* beta);

/*! returns version number of psychoacoustic model */
void nlame_lame_version_get_psy_num(int* major, int* minor, int* alpha, int* beta);


/* lame string get */

/*! type of string that should be returned by nlame_lame_string_get */
typedef enum
{
   nle_lame_string_url=0,
   nle_lame_string_features,
   nle_lame_string_compiler,
   nle_lame_string_cpu_features
} nlame_lame_string_type;


/*! returns the specified string */
const char* nlame_lame_string_get(nlame_lame_string_type type);


/* ------------------------------------------------------------- */


/* init and deinit functions */

/*! nlame internal settings struct */
struct nlame_struct;
/*! nlame encoder instance */
typedef struct nlame_struct nlame_instance_t;


/*! creates a new nlame instance */
nlame_instance_t* nlame_new();

/*! deletes a nlame instance */
void nlame_delete(nlame_instance_t* inst);


/* ------------------------------------------------------------- */


/* variables set and get functions */

/*! vbr mode values to use in a call to
    nlame_var_set_int(nle_var_vbr_mode,x) */
typedef enum
{
   nle_vbr_mode_off=0, /* same as vbr_off */
   nle_vbr_mode_old=2, /* same as vbr_rh, used in "normal" presets */
   nle_vbr_mode_new=4, /* same as vbr_mtrh, used in "fast" presets */
   nle_vbr_mode_abr=3  /* same as vbr_abr */
} nlame_vbr_mode;


/*! channel mode values to use in a call to
    nlame_var_set_int(nle_var_channel_mode,x) */
typedef enum
{
   nle_mode_stereo=0,
   nle_mode_joint_stereo,
   nle_mode_dual_channel,
   nle_mode_mono,
} nlame_channel_mode;


/*! preset types for variable nle_var_preset_vbr */
typedef enum
{
   /*! --alt-preset standard */
   nle_preset_standard=1001,
   /*! --alt-preset extreme */
   nle_preset_extreme=1002,
   /*! --alt-preset insane */
   nle_preset_insane=1003,
   /*! --alt-preset fast standard */
   nle_preset_standard_fast=1004,
   /*! --alt-preset fast extreme */
   nle_preset_extreme_fast=1005,
   /*! --alt-preset medium */
   nle_preset_medium=1006,
   /*! --alt-preset medium fast */
   nle_preset_medium_fast=1007,

   /* vbr presets */
   nle_preset_v0=500,
   nle_preset_v1=490,
   nle_preset_v2=480,
   nle_preset_v3=470,
   nle_preset_v4=460,
   nle_preset_v5=450,
   nle_preset_v6=440,
   nle_preset_v7=430,
   nle_preset_v8=420,
   nle_preset_v9=410,

   nle_preset_vx_first=410,
   nle_preset_vx_last=500,
   nle_preset_first=1001,
   nle_preset_last=1007,
} nlame_vbr_preset_type;


/*! type of integer variable that should be set/get */
typedef enum
{
   /* general settings */

   /*! the bitrate in kbps; must lie between 8 and 320 */
   nle_var_bitrate = 0,
   /*! specifies the qval to use; 0 < qval < 9 where 0 is best quality */
   nle_var_quality = 1,
   /*! output samplerate in Hz; default = 0, which means LAME picks best
       value based on the amount of compression. MPEG only allows:
       MPEG1    32, 44.1,   48khz
       MPEG2    16, 22.05,  24
       MPEG2.5   8, 11.025, 12 */
   nle_var_out_samplerate = 2,
   /*! channel mode lame will use to encode; use a value from the
       enum nlame_channel_mode; default: lame picks based on compression
       ratio and input channels */
   nle_var_channel_mode = 3,
   /*! the number of channels that will be encoded; either 1 or 2 */
   nle_var_num_channels = 4,
   /*! sample rate in Hz that the input samples have */
   nle_var_in_samplerate = 5,
   /*! when 1, produces free format bitstream; bitrate values are not constrained
       to fixed values in mpeg standard */
   nle_var_free_format = 6,
   /*! when 1, use a M/S mode with a switching threshold based on compression ratio */
   /* nle_var_auto_ms = 7, removed in lame-3.98 */
   /*! when 1, force M/S for all frames; default: 0 (disabled) */
   nle_var_force_ms = 8,

   /* VBR settings */

   /*! VBR mode; use one of the enum values in nlame_vbr_mode */
   nle_var_vbr_mode = 9,
   /*! VBR quality level; 0=highest, 9=lowest */
   nle_var_vbr_quality = 10,
   /*! minimum bitrate for VBR (ignored for vbr_abr), in kbps */
   nle_var_vbr_min_bitrate = 11,
   /*! maximum bitrate for VBR, in kbps */
   nle_var_vbr_max_bitrate = 12,
   /*! when set to 1, lame strictly enforces nle_var_vbr_min_bitrate;
       normally it will be violated for analog silence */
   nle_var_vbr_hard_min = 13,
   /*! the average bitrate to use when nle_var_vbr_mode is vbr_abr, in kbps */
   nle_var_abr_mean_bitrate = 14,
   /*! when 1, tells LAME to collect data for the VBR info tag
       after encoding, call nlame_write_vbr_infotag() to write VBR info tag */
   nle_var_vbr_generate_info_tag = 15,

   /* filter settings */

   /*! lowpass filter frequency, in Hz; default: 0 = lame chooses, -1 = disabled */
   nle_var_lowpass_freq = 16,
   /*! width of transition band, in Hz; default: one polyphase filter band */
   nle_var_lowpass_width = 17,
   /*! highpass filter frequency, in Hz; default: 0 = lame chooses, -1 = disabled */
   nle_var_highpass_freq = 18,
   /*! width of transition band, in Hz; default: one polyphase filter band */
   nle_var_highpass_width = 19,

   /* mp3 frame/stream settings */

   /*! when 1, sets the copyright flag in each frame */
   nle_var_copyright = 20,
   /*! when 1, sets the original flag in each frame */
   nle_var_original = 21,
   /*! enables error protection; uses 2 bytes from each frame for a CRC checksum */
   nle_var_error_protection = 22,
   /*! when 1, sets the private extension flag in each frame */
   nle_var_priv_extension = 23,
   /*! decides how to pad each frame; use values from enum nlame_padding_type */
   /* nle_var_padding_type = 24, removed in lame-3.98 */
   /*! when 1, uses a strict ISO conformance bitstream */
   nle_var_strict_iso = 25,

   /* ATH settings */

   /*! when 1, disables ATH */
   nle_var_ath_disable = 26,
   /*! when 1, only ATH is used for masking (psymodel output is ignored) */
   nle_var_ath_only = 27,
   /*! sets ATH type (formula) to use. must lie between 0 and 4; default: -1 */
   nle_var_ath_type = 28,
   /*! when 1, ATH is only used for short blocks */
   nle_var_ath_short = 29,

   /*! select ATH adaptive adjustment type */
   nle_var_athaa_type = 30,
   /*! select the loudness approximation used by the ATH adaptive auto-leveling */
   /* nle_var_athaa_loudapprox = 31, removed in lame-3.99.5 */
   /*! predictability limit (ISO tonality formula) */
   /* nle_var_athaa_cwlimit = 32, removed in lame-3.98 */

   /* misc. settings */

   /*! when 1, no short blocks are used to encode */
   nle_var_no_short_blocks = 33,
   /*! when 1, uses temporal masking effect */
   nle_var_use_temporal = 34,
   /*! when 1, allows blocktypes to differ between channels;
       default: 0 for jstereo, 1 for stereo */
   nle_var_allow_diff_short = 35,
   /*! when 1, input PCM is emphased PCM (for instance from one of the rarely
       emphased CDs), it is STRONGLY not recommended to use this, because
       psycho does not take it into account, and last but not least many decoders
       ignore these bits */
   nle_var_emphasis = 36,
   /*! when 1, disables the bit reservoir */
   nle_var_disable_reservoir = 37,

   /* alt preset settings */

   /*! sets optimized vbr-preset; allowed valus are from the enum
       nlame_vbr_preset_type; set only */
   nle_var_preset_vbr = 38,

   /*! sets alt-preset cbr preset values; allowed values: 80, 96, 112, 128,
       160, 192, 224, 256 and 320; set only */
   nle_var_preset_cbr = 39,
   /*! sets alt-preset abr preset values; allowed values range from 80 to 320;
       set only */
   nle_var_preset_abr = 40,

   /* read-only variables, don't set them! */

   /*! returns mpeg version of output; 0=MPEG-2, 1=MPEG-1, (2=MPEG-2.5); read only */
   nle_var_mpeg_version = 41,
   /*! encoder delay used by lame; in samples; read only */
   nle_var_encoder_delay = 42,
   /*! number of PCM samples buffered, but not yet encoded to mp3 data; read only */
   nle_var_samples_buffered = 43,
   /*! number of frames encoded so far; read only */
   nle_var_frames_encoded = 44,
   /*! size (bytes) of mp3 data buffered, but not yet encoded; read only */
   nle_var_size_mp3buffer = 45,

   /*! queries the quality value (nle_var_quality) for "high" quality, "-h" option */
   nle_var_quality_value_high = 46,
   /*! queries the quality value for "fast" quality, "-f" option */
   nle_var_quality_value_fast = 47,

   /* more variables, read/write */

   /*! when 1, replay gain analysis is done;
       1: same as --replaygain-fast and --replaygain-accurate
       0: same as --noreplaygain
   */
   nle_var_find_replay_gain = 48,

   /*! when 1, encoder also decodes frames on the fly, e.g. for replay gain analysis
       is used when --replaygain-accurate is specified at command line
       attention: this option currently doesn't anything, since the mpglib decoder
       has to be built into LAME to get this to work, which isn't currently compiled
       in.
   */
   nle_var_decode_on_the_fly = 49,

   /*! number of samples in one frame; read only */
   nle_var_framesize = 50,

} nlame_var_int_type;


/*! type of float variable that should be set/get */
typedef enum
{
   /*! scale value for all input samples; 1.0f is no change,
       0.0f is default */
   nle_var_float_scale = 0,
   /*! compression ratio of input to output. use instead of nle_var_bitrate */
   nle_var_float_compression_ratio = 1,

   /*! value in dB the ATH will be lowered */
   nle_var_float_ath_lower = 2,
   /*! adjust (in dB) the point below which adaptive ATH level adjustment occurs */
   nle_var_float_athaa_sensitivity = 3,

   /*! interchannel masking ratio; default: 0.0f */
   nle_var_float_interch = 4,

   /*! VBR quality level; float version; range: [0; 10[; 0=highest, 10=lowest */
   nle_var_float_vbr_quality = 5,

} nlame_var_float_type;


/*! sets an integer variable */
int nlame_var_set_int(nlame_instance_t* inst, nlame_var_int_type type, int value);

/*! returns the value of the integer variable */
int nlame_var_get_int(nlame_instance_t* inst, nlame_var_int_type type);


/*! sets a float variable */
int nlame_var_set_float(nlame_instance_t* inst, nlame_var_float_type type, float value);

/*! returns the value of the float variable */
float nlame_var_get_float(nlame_instance_t* inst, nlame_var_float_type type);


/*! inits encoder based on settings made */
int nlame_init_params(nlame_instance_t* inst);


/* ------------------------------------------------------------- */


/* callback functions */


/*! callback function type */
typedef void (*nlame_callback_func)(const char* , va_list);

/*! type of callback the user can set */
typedef enum
{
   nle_callback_error,
   nle_callback_debug,
   nle_callback_message,
} nlame_callback_type;


/*! sets a callback function for a specific type; if you pass NULL, the
    channel will be quiet.

    to install your own (e.g. debug) callback function, create a function like

    void my_debugf(const char* format, va_list ap)
    {
       (void) vfprintf(stdout, format, ap);
    }

    then call nlame_callback_set(my_inst, nle_cback_debug, my_debugf); */

void nlame_callback_set(nlame_instance_t* inst, nlame_callback_type type, nlame_callback_func func);


/* ------------------------------------------------------------- */


/* encode functions */


/*! maximum size of mp3buffer needed if you encode at most 1152 samples */
const int nlame_const_maxmp3buffer = 16384;


/*! type of samples in buffer in call to nlame_encode_buffer */
typedef enum
{
   /*! buffer consists of 16 bit short int's */
   nle_buffer_short=0,
   /*! buffer contains 32 bit int's */
   nle_buffer_int,
   /*! buffer contains float's (allowed range: -32767.0 to 32768.0) */
   nle_buffer_float,
   /*! buffer contains 32 bit long's */
   nle_buffer_long,
} nlame_encode_buffer_type;



/*! encodes samples that are separated into a left and right buffer */
/*! \param inst encoder instance
    \param buftype type of buffer that is passed in buffer_l and buffer_r
    \param buffer_l pointer to left buffer data; type must correspond to buftype
    \param buffer_r pointer to right buffer data
    \param nsamples number of samples (not bytes!) in each buffer
    \param mp3buf buffer that receives mp3 frame data
    \param mp3buf_size size of mp3 output buffer in bytes
*/
int nlame_encode_buffer( nlame_instance_t* inst,
   nlame_encode_buffer_type buftype,
   const void* buffer_l,
   const void* buffer_r,
   const int nsamples,
   unsigned char* mp3buf,
   const unsigned int mp3buf_size );


/*! encodes samples in a single channel; note that channel number has to be
    set to 1 in variable nle_var_num_channels */
/*! \param inst encoder instance
    \param buftype type of buffer that is passed in buffer_m
    \param buffer_m pointer to mono buffer data; type must correspond to buftype
    \param nsamples number of samples (not bytes!) in buffer
    \param mp3buf buffer that receives mp3 frame data
    \param mp3buf_size size of mp3 output buffer in bytes
*/
int nlame_encode_buffer_mono( nlame_instance_t* inst,
   nlame_encode_buffer_type buftype,
   const void* buffer_m,
   const int nsamples,
   unsigned char* mp3buf,
   const unsigned int mp3buf_size );


/*! encodes samples that are interleaved in a single buffer */
/*! \param inst encoder instance
    \param buftype type of buffer that is passed in buffer_i
           currently only nle_buffer_short and nle_buffer_int are supported.
    \param buffer_i pointer to buffer with interleaved samples;
        sample order: <sample-left> <sample-right> <sample-left> ...
    \param nsamples number of samples per channel (not interleaved samples
        nor bytes!) per channel in buffer
    \param mp3buf buffer that receives mp3 frame data
    \param mp3buf_size size of mp3 output buffer in bytes
*/
int nlame_encode_buffer_interleaved( nlame_instance_t* inst,
   nlame_encode_buffer_type buftype,
   const void* buffer_i,
   const int nsamples,
   unsigned char* mp3buf,
   const unsigned int mp3buf_size );


/*! flushes all pending output */
/*! \param inst encoder instance
    \param mp3buf buffer that receives mp3 frame data
    \param mp3buf_size size of mp3 output buffer in bytes
*/
int nlame_encode_flush( nlame_instance_t* inst,
   unsigned char* mp3buf,
   const unsigned int mp3buf_size );

/*! flushes all pending output, and prepares the stream for new audio input;
    doesn't reinitialize the bitstream (see nlame_reinit_bitstream for more) */
/*! \param inst encoder instance
    \param mp3buf buffer that receives mp3 frame data
    \param mp3buf_size size of mp3 output buffer in bytes
*/
int nlame_encode_flush_nogap( nlame_instance_t* inst,
   unsigned char* mp3buf,
   const unsigned int mp3buf_size );

/*! reinitializes the bitstream; this is needed when doing nogap encoding;
    when new id3v2 tag infos are set, call this method to write tags to
    buffer */
/*! \param inst encoder instance
*/
int nlame_reinit_bitstream( nlame_instance_t* inst );


/* ------------------------------------------------------------- */


/* misc. functions */


/*! writes VBR info tag to an open file descriptor fd */
/*! These calls perform forward and backwards seeks, so make
    sure fd is a real file. Make sure nlame_encode_flush has been called,
    and all mp3 data has been written to the file before calling this
    function.

    NOTE:
    if VBR tags are turned off by the user, or turned off by LAME because
    the output is not a regular file, this call does nothing
*/
void nlame_write_vbr_infotag( nlame_instance_t* inst, FILE* fd );


/*! type of histogram to get in call to nlame_histogram_get */
typedef enum
{
   /*! gets a bitrate histogram; the array must have 14 entries, which are the
       different possible bitrates in the file */
   nle_hist_bitrate=0,
   /*! returns the bitrates in kbps used for the histogram; the array must
       have 14 entries */
   nle_hist_kbps,
   /*! returns a stereo mode histogram; the array must have 4 entries, and
       they specify MPEG stereo modes;
        0: LR   number of left-right encoded frames
        1: LR-I number of left-right and intensity encoded frames
        2: MS   number of mid-side encoded frames
        3: MS-I number of mid-side and intensity encoded frames */
   nle_hist_stereo_mode,
   /*! returns a stereo mode bitrate histogram; the array must be of type
       [14][4] and receives all infos from nle_hist_bitrate and
       nle_hist_stereo_mode combined */
   nle_hist_bitrate_stereo_mode

} nlame_histogram_type;


/*! returns a histogram determined by nlame_bitrate_hist_type */
/*! note that the histogram must have as much array slots as specified
    in the description of nlame_bitrate_hist_type;
    attention: don't call them after lame_encode_finish */
void nlame_histogram_get( nlame_instance_t* inst,
   nlame_histogram_type type, int* histogram );



/*! initializes id3 tag writing */
void nlame_id3tag_init( nlame_instance_t* inst,
   int bWriteId3v1Tag, int bWriteId3v2Tag, unsigned int uiV2ExtraPadSize);

/*! indicates which id3 tag field is to be set in the function
    nlame_id3tag_setfield_latin1()
*/
enum nlame_id3tag_field
{
   nif_title = 0,
   nif_artist = 1,
   nif_album = 2,
   nif_year = 3,
   nif_comment = 4,
   nif_track = 5,
   nif_genre = 6,
};

/*! writes latin1 string text field to stream */
/*! must be called before any encoding happens */
void nlame_id3tag_setfield_latin1( nlame_instance_t* inst,
   enum nlame_id3tag_field field, const char* text);


/*! returns the version number of the nLAME API */
/*! when this function is not present, assume version 0 of this API */
int nlame_get_api_version();


/*! compare this version constant with the value from
    nlame_get_api_version() to check if the API of the nLAME.dll you're
    actually using is new enough to support the features you need. See
    the version history at the beginning of this file.
*/
#define NLAME_CURRENT_API_VERSION 4



/* ------------------------------------------------------------- */

#ifdef __cplusplus
}
#endif


/*@} */

#endif
