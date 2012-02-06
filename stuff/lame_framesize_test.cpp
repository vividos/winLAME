#include "nlame.h"
#include "sndfile.h"
#include "encoder/wlSampleContainer.h"

void TestEncoding()
{
   SNDFILE* sndfile;
   SF_INFO sfinfo;

   const int sndfile_inbufsize = 16*4096;

   // sndfile init
   sndfile = sf_open("D:\\projekte\\winLAME\\test\\TestWave.wav",SFM_READ,&sfinfo);

   short* inbuffer = new short[sndfile_inbufsize*sfinfo.channels];


   // LAME init
   nlame_instance_t* inst1 = nlame_new();
   nlame_instance_t* inst2 = nlame_new();
   {
      nlame_var_set_int(inst1, nle_var_in_samplerate, sfinfo.samplerate);
      nlame_var_set_int(inst2, nle_var_in_samplerate, sfinfo.samplerate);

      nlame_var_set_int(inst1, nle_var_num_channels, sfinfo.channels);
      nlame_var_set_int(inst2, nle_var_num_channels, sfinfo.channels);

      // ABR, 64kbit
      nlame_var_set_int(inst1, nle_var_vbr_mode, nle_vbr_mode_abr);
      nlame_var_set_int(inst2, nle_var_vbr_mode, nle_vbr_mode_abr);

      nlame_var_set_int(inst1, nle_var_abr_mean_bitrate, 64);
      nlame_var_set_int(inst2, nle_var_abr_mean_bitrate, 64);

      // misc settings
      nlame_var_set_int(inst1,nle_var_find_replay_gain,1);
      nlame_var_set_int(inst2,nle_var_find_replay_gain,1);

      nlame_init_params(inst1);
      nlame_init_params(inst2);
   }

   const int mp3_bufsize = 4096;

   short inbuf1[sndfile_inbufsize], inbuf2[sndfile_inbufsize];

   unsigned char mp3buf1[mp3_bufsize], mp3buf2[mp3_bufsize];

   int buf1_filled = 0, buf2_filled = 0;
   int mp3buf1_filled = 0, mp3buf2_filled = 0;

   for(;;)
   {
      int min_size_to_read = sndfile_inbufsize - __max(buf1_filled, buf2_filled);

      if (min_size_to_read == 0)
         _asm nop;

      sf_count_t in_ret = sf_readf_short(sndfile,inbuffer,min_size_to_read);
      if (in_ret <= 0)
         break;

      int ret1, ret2;
/ *
      memset(mp3buf1, 0xcc, mp3_bufsize);
      memset(mp3buf2, 0xcc, mp3_bufsize);
* /

      // move samples to input buffer
      memcpy(inbuf1 + buf1_filled, inbuffer, in_ret*sizeof(*inbuf1));
      buf1_filled += in_ret;

      // move samples to input buffer
      memcpy(inbuf2 + buf2_filled, inbuffer, in_ret*sizeof(*inbuf2));
      buf2_filled += in_ret;

      CString cszText;
      cszText.Format("bufsize1: %04x - bufsize2: %04x\n", buf1_filled, buf2_filled);
      OutputDebugString(cszText);

      // encode 512 samples until a frame comes out
      const int framesize1 = 576+10;
      do
      {
         ret1 = ::nlame_encode_buffer_interleaved(inst1, nle_buffer_short,
            inbuf1, framesize1, mp3buf1+mp3buf1_filled, mp3_bufsize-mp3buf1_filled);

//         if (ret1 == 0)
//            OutputDebugString("buf1: need more samples\n");

         // copy input samples to front
         memmove(inbuf1, inbuf1+framesize1, framesize1*sizeof(*inbuf1));

         buf1_filled -= framesize1;
      }
      while (buf1_filled >= framesize1 && ret1 == 0);

      // encode 576 samples until a frame comes out
      const int framesize2 = 576;
      do
      {
         ret2 = ::nlame_encode_buffer_interleaved(inst2, nle_buffer_short,
            inbuf2, framesize2, mp3buf2+mp3buf2_filled, mp3_bufsize-mp3buf1_filled);

//         if (ret2 == 0)
//            OutputDebugString("buf2: need more samples\n");

         // copy input samples to front
         memmove(inbuf2, inbuf2+framesize2, framesize2*sizeof(*inbuf2));

         buf2_filled -= framesize2;
      }
      while (buf2_filled >= framesize2 && ret2 == 0);

      cszText.Format("comparing %u bytes with %u bytes\n", ret1, ret2);
      OutputDebugString(cszText);

      mp3buf1_filled += ret1;
      mp3buf2_filled += ret2;

      int min_compare_size = __min(mp3buf1_filled, mp3buf2_filled);

      if (min_compare_size > 0)
      {
         cszText.Format("mp3buf1: %02x %02x %02x %02x %02x %02x %02x %02x ...\n",
            mp3buf1[0], mp3buf1[1], mp3buf1[2], mp3buf1[3], mp3buf1[4], mp3buf1[5], mp3buf1[6], mp3buf1[7]);
         OutputDebugString(cszText);

         cszText.Format("mp3buf2: %02x %02x %02x %02x %02x %02x %02x %02x ...\n",
            mp3buf2[0], mp3buf2[1], mp3buf2[2], mp3buf2[3], mp3buf2[4], mp3buf2[5], mp3buf2[6], mp3buf2[7]);
         OutputDebugString(cszText);

         if (memcmp(mp3buf1, mp3buf2, min_compare_size) != 0)
         {
            unsigned char* p1=mp3buf1, *p2=mp3buf2;
            unsigned int pos = 0;
            while(*(p1++) == *(p2++) && pos < min_compare_size)
               pos++;

            p1--; p2--;

            cszText.Format("diff at %04x:\n %02x %02x %02x %02x\n %02x %02x %02x %02x\n",
               pos, p1[0], p1[1], p1[2], p1[3], p2[0], p2[1], p2[2], p2[3]);
            OutputDebugString(cszText);

            _asm nop;
         }

         // move uncompared bytes to front, buf1
         int left_bytes1 = mp3buf1_filled - min_compare_size;
         if (left_bytes1 > 0)
            memmove(mp3buf1, mp3buf1+min_compare_size, left_bytes1);
         mp3buf1_filled -= min_compare_size;

         // move uncompared bytes to front, buf2
         int left_bytes2 = mp3buf2_filled - min_compare_size;
         if (left_bytes2 > 0)
            memmove(mp3buf2, mp3buf2+min_compare_size, left_bytes2);
         mp3buf2_filled -= min_compare_size;
      }
   }

   nlame_delete(inst1);
   nlame_delete(inst2);

   sf_close(sndfile);

   delete[] inbuffer;
}
