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
 * $Id: dither.h,v 1.4 2002/09/03 18:43:49 vividos Exp $
 */
/*! \file dither.h

   \brief linear dither functions

*/

# ifndef AUDIO_H
# define AUDIO_H

# include "mad.h"

#ifdef __cplusplus
extern "C"
{
#endif

enum audio_mode {
  AUDIO_MODE_ROUND,
  AUDIO_MODE_DITHER
};

struct audio_stats {
  unsigned long clipped_samples;
  mad_fixed_t peak_clipping;
  mad_fixed_t peak_sample;
};

struct audio_dither {
  mad_fixed_t error[3];
  mad_fixed_t random;
};

void audio_init_stats(struct audio_stats *);

void audio_init_dither(struct audio_dither *);

signed long audio_linear_dither(unsigned int, mad_fixed_t,
  struct audio_dither *, struct audio_stats *);

#ifdef __cplusplus
} // extern "C"
#endif

# endif
