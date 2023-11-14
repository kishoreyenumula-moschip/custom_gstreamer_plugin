#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <gst/video/gstvideometa.h>
#include <gst/video/gstvideopool.h>
#include <gst/gst.h>
#include <gst/base/base.h>
#include <gst/controller/controller.h>
#include<gst/video/video.h>
#include<stdlib.h>
#include "gstresolution.h"


#define DEF_WIDTH 0
#define DEF_HEIGHT 0
#define DEF_X   0
#define DEF_Y   0

#define TLX     0
#define TLY     0
#define TRX     320
#define RTY     0
#define BLX     0
#define BLY     240
#define BRX     320
#define BRY     240
#define SWIDTH  1920
#define SHEIGHT 1080
#define MAXRANGE 2147483647

