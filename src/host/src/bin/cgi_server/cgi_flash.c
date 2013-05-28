/****************************************************************************\
*  Copyright C 2013 Stretch, Inc. All rights reserved. Stretch products are  *
*  protected under numerous U.S. and foreign patents, maskwork rights,       *
*  copyrights and other intellectual property laws.                          *
*                                                                            *
*  This source code and the related tools, software code and documentation,  *
*  and your use thereof, are subject to and governed by the terms and        *
*  conditions of the applicable Stretch IDE or SDK and RDK License Agreement *
*  (either as agreed by you or found at www.stretchinc.com). By using these  *
*  items, you indicate your acceptance of such terms and conditions between  *
*  you and Stretch, Inc. In the event that you do not agree with such terms  *
*  and conditions, you may not use any of these items and must immediately   *
*  destroy any copies you have made.                                         *
\****************************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>

#include <mtd/mtd-user.h>
#include <mtd/jffs2-user.h>

#include "cgi_flash.h"

/* This *must* be C code. jffs2.h conveniently declares a struct member with name 'new'. Uggh! */

/* This makes it not thread_safe */
static char error_msg[200];

/* Erase flash device. Return error message or NULL if there was no errors */
const char* erase_flash(const char* mtd_device) {
	mtd_info_t meminfo;
	erase_info_t erase;
	int fd;
    
	if ((fd = open(mtd_device, O_RDWR)) < 0) {
		snprintf(error_msg, sizeof error_msg, "%s: unable to open, %s", mtd_device, strerror(errno));
		return error_msg;
	}


	if (ioctl(fd, MEMGETINFO, &meminfo) != 0) {
		snprintf(error_msg, sizeof error_msg, "%s: unable to get MTD device info", mtd_device);
		return error_msg;
	}

	erase.length = meminfo.erasesize;
	for (erase.start = 0; erase.start < meminfo.size; erase.start += meminfo.erasesize) {
		if (ioctl(fd, MEMERASE, &erase) != 0) {
			snprintf(error_msg, sizeof error_msg, "%s: MTD Erase failure: %s\n", mtd_device, strerror(errno));
			return error_msg;
		}
	}
	return NULL;
}

/* Write a file to flash. Return error message if there was an error and NULL otherwise */
const char* write_flash(const char* filename, const char* mtd_device) {
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
		snprintf(error_msg, sizeof error_msg, "%s: unable to open, %s", filename, strerror(errno));
		return error_msg;
    }
    int md = open(mtd_device, O_WRONLY);
    if (md < 0) {
		snprintf(error_msg, sizeof error_msg, "%s: unable to open, %s", mtd_device, strerror(errno));
		return error_msg;
    }
    const int SIZE = 16 * 1024;
    char* buffer = (char*) malloc(SIZE);
    if (buffer == 0) {
		snprintf(error_msg, sizeof error_msg, "unable to allocate memory");
		return error_msg;
    }
    int error_flag = 0;
    do {
        int read_size = read(fd, buffer, SIZE);
        if (read_size == 0) 
            break;
        else if (read_size < 0) {
            snprintf(error_msg, sizeof error_msg, "%s: error reading, %s", filename, strerror(errno));
            return error_msg;
        }
        int write_size = write(md, buffer, read_size);
        if (write_size < 0) {
            snprintf(error_msg, sizeof error_msg, "%s: error writing, %s", mtd_device, strerror(errno));
            return error_msg;
        }
        if (read_size != write_size) {
            snprintf(error_msg, sizeof error_msg, "%s read_size (%d) != %s write size (%d)", filename, read_size, 
                     mtd_device, write_size);
        }
    } while (1);
    close(fd);
    close(md);
    free(buffer);
    return error_flag ? error_msg : NULL;
}
