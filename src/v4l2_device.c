#include <v4l2_device.h>
#include <event_loop.h>

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <linux/videodev2.h>

#define MAX_BUFFERS 20

struct v4l2_device {
	int fd;
	struct event_loop_request* event_loop_request;
	struct {
		void* start;
		size_t length;
	}* buffers;
	unsigned int buffers_size;
};

static int v4l2_device_stream_on(struct v4l2_device* v4l2_device) {
	int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	return ioctl(v4l2_device->fd, VIDIOC_STREAMON, &type);
}

static void v4l2_device_stream_off(struct v4l2_device* v4l2_device) {
	int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	ioctl(v4l2_device->fd, VIDIOC_STREAMOFF, &type);
}

static void v4l2_device_stop_capture(struct v4l2_device* v4l2_device, unsigned int num) {
	struct v4l2_requestbuffers reqbuf;

	memset(&reqbuf, 0, sizeof(reqbuf));
	reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	reqbuf.memory = V4L2_MEMORY_MMAP;
	reqbuf.count = num;

	v4l2_device_stream_off(v4l2_device);

	ioctl(v4l2_device->fd, VIDIOC_REQBUFS, &reqbuf);

	while (num--) {
		munmap((v4l2_device->buffers+num)->start, (v4l2_device->buffers+num)->length);
	}

	if (v4l2_device->buffers)
		free(v4l2_device->buffers);
}

static int v4l2_device_setup_capture(struct v4l2_device* v4l2_device) {
	struct v4l2_requestbuffers reqbuf;
	unsigned int i;

	memset(&reqbuf, 0, sizeof(reqbuf));
	reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	reqbuf.memory = V4L2_MEMORY_MMAP;
	reqbuf.count = MAX_BUFFERS;

	if (ioctl(v4l2_device->fd, VIDIOC_REQBUFS, &reqbuf) == -1)
		return -1;

	v4l2_device->buffers_size = reqbuf.count;
	v4l2_device->buffers = calloc(v4l2_device->buffers_size, sizeof(*(v4l2_device->buffers)));
	if (v4l2_device->buffers == NULL) {
		v4l2_device_stop_capture(v4l2_device, 0);
		return -1;
	}

	for (i = 0; i < reqbuf.count; ++i) {
		struct v4l2_buffer buffer;

		memset (&buffer, 0, sizeof (buffer));
		buffer.type = reqbuf.type;
		buffer.memory = V4L2_MEMORY_MMAP;
		buffer.index = i;

		if (ioctl(v4l2_device->fd, VIDIOC_QUERYBUF, &buffer) == -1) {
			v4l2_device_stop_capture(v4l2_device, i);
			return -1;
		}

		(v4l2_device->buffers+i)->length = buffer.length;
		(v4l2_device->buffers+i)->start = mmap(NULL, buffer.length, PROT_READ | PROT_WRITE, MAP_SHARED, v4l2_device->fd, buffer.m.offset);
		if ((v4l2_device->buffers+i)->start == MAP_FAILED) {
			v4l2_device_stop_capture(v4l2_device, i);
			return -1;
		}

		if (ioctl(v4l2_device->fd, VIDIOC_QBUF, &buffer) == -1) {
			v4l2_device_stop_capture(v4l2_device, i+1);
			return -1;
		}
	}

	if (v4l2_device_stream_on(v4l2_device) == -1) {
		v4l2_device_stop_capture(v4l2_device, v4l2_device->buffers_size);
		return -1;
	}

	return 0;
}

struct v4l2_device* init_v4l2_device(const char* filename) {
	struct v4l2_device* v4l2_device = NULL;

	v4l2_device = malloc(sizeof(struct v4l2_device));
	if (v4l2_device == NULL)
		return NULL;

	if ((v4l2_device->fd = open(filename, O_NONBLOCK | O_RDWR)) == -1) {
		free(v4l2_device);
		return NULL;
	}

	v4l2_device->event_loop_request = NULL;
	v4l2_device->buffers = NULL;

	if (v4l2_device_setup_capture(v4l2_device) == -1) {
		close(v4l2_device->fd);
		free(v4l2_device);
		return NULL;
	}

	return v4l2_device;
}

void free_v4l2_device(struct v4l2_device* v4l2_device) {
	if (v4l2_device->buffers) {
		v4l2_device_stop_capture(v4l2_device, v4l2_device->buffers_size);
	}
	if (v4l2_device->event_loop_request) {
		event_loop_del_request(v4l2_device->event_loop_request);
	}
	close(v4l2_device->fd);
	free(v4l2_device);
}

static void v4l2_device_handle_rx(void* data) {
	struct v4l2_device* v4l2_device = data;
	struct v4l2_buffer buffer;

	memset (&buffer, 0, sizeof (buffer));
	buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buffer.memory = V4L2_MEMORY_MMAP;

	if (ioctl(v4l2_device->fd, VIDIOC_DQBUF, &buffer) == 0) {
//		printf("%d:%06d\n", buffer.timestamp.tv_sec, buffer.timestamp.tv_usec);
		ioctl(v4l2_device->fd, VIDIOC_QBUF, &buffer);
	}
}

int v4l2_device_attach_event_loop(struct v4l2_device* v4l2_device, struct event_loop* event_loop) {
	if (v4l2_device->event_loop_request != NULL)
		return -1;

	v4l2_device->event_loop_request = event_loop_add_request(event_loop, v4l2_device->fd, v4l2_device_handle_rx, v4l2_device);
	if (v4l2_device->event_loop_request == NULL)
		return -1;

	return 0;
}
