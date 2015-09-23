/*
 * slideshow.h
 *
 *  Created on: Sep 16, 2015
 *      Author: Robo
 */

#ifndef EPD_APP_SLIDESHOW_H_
#define EPD_APP_SLIDESHOW_H_

#include "../pl/platform.h"

int show_image(struct pl_platform *plat, const char *dir,
		      const char *file);

int show_image_directstream(struct pl_platform *plat, unsigned char **data, int dataLen, file_streaming_stage_t stage);


#endif /* EPD_APP_SLIDESHOW_H_ */
