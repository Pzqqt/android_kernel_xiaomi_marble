/*
 * Copyright (c) 2019-2020 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * DOC: cfr capture test application
 * This file provides test application to dump cfr capture from driver
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdint.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>

#define CFR_DUMP_STREAMFS_FILE "/sys/kernel/debug/qdf/cfr%s/cfr_dump0"
#define CFR_DUMP_FILE "/tmp/cfr_dump_%s_%s.bin"

#define MAX_FILE_SIZE          (8 * 1024 * 1024)
#define MAX_CAPTURE_SIZE       (4096)
static char rbuffer[MAX_CAPTURE_SIZE];

int stop_capture;

void print_usage(char *argv[])
{
	printf("Usage:cfr_test_app -i <interfacename>\n");
}

void streamfs_read_handler(int sfd, int cfd)
{
	int rlen = 0, retval = 0;

	memset(rbuffer, 0, sizeof(rbuffer));
	rlen = read(sfd, rbuffer, sizeof(rbuffer));
	if (rlen <= 0)
		return;

	if (lseek(cfd, 0, SEEK_CUR) + rlen > MAX_FILE_SIZE) {
		retval = lseek(cfd, 0, SEEK_SET);
		if (retval < 0) {
			perror("lseek()");
			exit(EXIT_FAILURE);
		}
	}

	write(cfd, rbuffer, rlen);
}

int start_cfr_capture_daemon(int streamfs_fd, int cfr_dump_fd)
{
	fd_set sfs_fdset;
	int maxfd = 0, retval = 0;

	FD_ZERO(&sfs_fdset);
	FD_SET(streamfs_fd, &sfs_fdset);
	maxfd = streamfs_fd;

	while (!stop_capture) {
		retval = select(maxfd + 1, &sfs_fdset, NULL, NULL, NULL);
		if (retval < 0) {
			perror("select()");
			exit(EXIT_FAILURE);
		}

		if (FD_ISSET(streamfs_fd, &sfs_fdset))
			streamfs_read_handler(streamfs_fd, cfr_dump_fd);
	}

	close(streamfs_fd);
	close(cfr_dump_fd);
	return 0;
}

int open_streamfs_file(char *iface)
{
	int fd = -1;
	char filename[128];

	snprintf(filename, sizeof(filename), CFR_DUMP_STREAMFS_FILE, iface);
	fd = open(filename, O_RDONLY);
	return fd;
}

int initialize_cfr_dump_file(char *iface)
{
	int fd = -1;
	static char filename[128];
	char time[50] = {0};
	struct timeval tv = {0};
	time_t curtime = 0;
	struct tm *tm_val = NULL;

	gettimeofday(&tv, NULL);
	curtime = tv.tv_sec;
	tm_val = localtime(&curtime);
	if (tm_val) {
		strftime(time, 50, "%Y_%m_%d_%T", tm_val);
		snprintf(filename, sizeof(filename), CFR_DUMP_FILE, iface, time);
		fd = open(filename, O_WRONLY | O_CREAT);
	} else {
		perror("Unable to get time value to generate filename \n");
	}

	return fd;
}

/* Signal Hnadler to overide CTRL+C */
void sig_handler(int signum)
{
	stop_capture = 1;
}

int main(int argc, char *argv[])
{
	int option = 0;
	char *iface = NULL;
	int streamfs_fd = -1, cfr_dump_fd = -1;

	while ((option = getopt(argc, argv, "i:")) != -1) {
		switch (option) {
		case 'i':
			iface = optarg;
			break;
		default:
			printf("Invalid argument\n");
			print_usage(argv);
			exit(EXIT_FAILURE);
			break;
		}
	}

	if (iface == NULL) {
		printf("Invalid interface option\n");
		exit(EXIT_FAILURE);
	}

	streamfs_fd = open_streamfs_file(iface);

	if (streamfs_fd < 0) {
		printf("Invalid interface: %s, CFR capture file not found\n", iface);
		exit(EXIT_FAILURE);
	}

	cfr_dump_fd = initialize_cfr_dump_file(iface);

	if (cfr_dump_fd < 0) {
		printf(" Could not open CFR dump file for write\n");
		close(streamfs_fd);
		exit(EXIT_FAILURE);
	}

	printf("Starting CFR capture deamon, Press CTRL+C to exit \n");
	signal(SIGINT, sig_handler);
	start_cfr_capture_daemon(streamfs_fd, cfr_dump_fd);
	return 0;
}
