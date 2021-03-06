/*
 * mali-memtester version 0.1
 * 
 * Memory Testing tool, based on https://github.com/ssvb/lima-memtester but
 * using MALI instead of LIMA
 * 
 * https://github.com/dimitar-kunchev/mali-memtester
 * 
 * Copyright (C) 2018 Dimitar Kunchev <d.kunchev@gmail.com>
 * Licensed under the terms of the GNU General Public License version 2 (only).
 * See the file COPYING for details.
 *
 */

#include <pthread.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include "textured-cube.h"
#include "memtester-4.3.0/memtester.h"

int threads_stop = 0;

void *fb_unblank_thread(void *data) {
	int fd, ret;

	fd = open("/dev/fb0", O_RDWR);
	assert(fd != -1);

	while (!threads_stop) {
		ret = ioctl(fd, FBIOBLANK, FB_BLANK_UNBLANK);
		assert(!ret);
		sleep(1);
	}

	close(fd);
	return 0;
}

static void *cube_thread(void *threadid)
{
	textured_cube_main(&threads_stop);
	if (threads_stop == 0) {
		/* If we reach here, something bad has happened */
		abort();
	}
	return NULL;
}

int main(int argc, char **argv) {
	printf("This is a simple textured cube demo from the mali driver and\n");
	printf("a memtester. Both combined in a single program. The mali400\n");
	printf("hardware is only used to stress RAM in the background. But\n");
	printf("this happens to significantly increase chances of exposing\n");
	printf("memory stability related problems.\n\n");

	if (argc > 1) {
		pthread_t th1, th2;
		pthread_create(&th1, NULL, cube_thread, NULL);
		pthread_create(&th2, NULL, fb_unblank_thread, NULL);
		sleep(1);
		printf("\n");

		int ec = memtester_main(argc, argv);

		// printf("Memtester completed, %d\n", ec);
		threads_stop = 1;
		pthread_join(th2, NULL);
		// printf("TH2 done\n");
		pthread_join(th1, NULL);
		// printf("TH1 done\n");
		return ec;
	} else {
		return 0;
	}
}
