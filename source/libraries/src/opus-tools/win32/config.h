#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

#define OUTSIDE_SPEEX         1
#define OPUSTOOLS             1

#define inline __inline
#define alloca _alloca
#define getpid _getpid
#define USE_ALLOCA            1
#define FLOATING_POINT        1
#define SPX_RESAMPLE_EXPORT
#define __SSE__

#define RANDOM_PREFIX winLAME

#define PACKAGE_NAME "opus-tools"
#include "version.h"

typedef uint16_t ogg_uint16_t;

#endif /* CONFIG_H */
