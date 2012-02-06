/*
 * mad - MPEG audio decoder
 * Copyright (C) 2000-2001 Robert Leslie
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id: dither.c,v 1.6 2005/02/22 19:14:07 vividos Exp $
 */
/*! \file dither.c

   \brief contains implementation of the linear dither function

*/

# include <string.h>

# include "dither.h"
# include "mad.h"

# if defined(_MSC_VER)
#  pragma warning(disable: 4550)  /* expression evaluates to a function which
                                     is missing an argument list */
# endif

void audio_init_dither(struct audio_dither *dither)
{
  dither->error[0] = dither->error[1] = dither->error[2] = dither->random = 0;
}

void audio_init_stats(struct audio_stats *stats)
{
  stats->clipped_samples = 0;
  stats->peak_clipping   = 0;
  stats->peak_sample     = 0;
}

/*
 * NAME:        prng()
 * DESCRIPTION:        32-bit pseudo-random number generator
 */
static
unsigned long prng(unsigned long state)
{
  return (state * 0x0019660dL + 0x3c6ef35fL) & 0xffffffffL;
}

/*
 * NAME:        audio_linear_dither()
 * DESCRIPTION:        generic linear sample quantize and dither routine
 */
signed long audio_linear_dither(unsigned int bits, mad_fixed_t sample,
                                struct audio_dither *dither,
                                struct audio_stats *stats)
{
  unsigned int scalebits;
  mad_fixed_t output, mask, random;

  enum {
    MIN = -MAD_F_ONE,
    MAX =  MAD_F_ONE - 1
  };

  /* noise shape */
  sample += dither->error[0] - dither->error[1] + dither->error[2];

  dither->error[2] = dither->error[1];
  dither->error[1] = dither->error[0] / 2;

  /* bias */
  output = sample + (1L << (MAD_F_FRACBITS + 1 - bits - 1));

  scalebits = MAD_F_FRACBITS + 1 - bits;
  mask = (1L << scalebits) - 1;

  /* dither */
  random  = prng(dither->random);
  output += (random & mask) - (dither->random & mask);

  dither->random = random;

  /* clip */
  if (output >= stats->peak_sample) {
    if (output > MAX) {
      ++stats->clipped_samples;
      if (output - MAX > stats->peak_clipping)
        stats->peak_clipping = output - MAX;

      output = MAX;

      if (sample > MAX)
        sample = MAX;
    }
    stats->peak_sample = output;
  }
  else if (output < -stats->peak_sample) {
    if (output < MIN) {
      ++stats->clipped_samples;
      if (MIN - output > stats->peak_clipping)
        stats->peak_clipping = MIN - output;

      output = MIN;

      if (sample < MIN)
        sample = MIN;
    }
    stats->peak_sample = -output;
  }

  /* quantize */
  output &= ~mask;

  /* error feedback */
  dither->error[0] = sample - output;

  /* scale */
  return output >> scalebits;
}
