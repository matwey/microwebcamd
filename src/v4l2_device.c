#include <v4l2_device.h>
#include <event_loop.h>
#include <frame_request.h>
#include <frame.h>

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
	struct event_loop_request event_loop_request;
	struct frame* frames;
	size_t frames_num;
	struct frame_request_queue queue;
};

static int v4l2_device_stream_on(struct v4l2_device* v4l2_device) {
	int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (ioctl(v4l2_device->fd, VIDIOC_STREAMON, &type) == -1)
		goto err;

	if (event_loop_add_request(v4l2_device->event_loop_request.event_loop, &v4l2_device->event_loop_request) == -1)
		goto err_event_loop_add_request;

	return 0;

err_event_loop_add_request:
	ioctl(v4l2_device->fd, VIDIOC_STREAMOFF, &type);
err:
	return -1;
}

static void v4l2_device_stream_off(struct v4l2_device* v4l2_device) {
	int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	event_loop_del_request(&v4l2_device->event_loop_request);
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
		munmap((v4l2_device->frames+num)->data, (v4l2_device->frames+num)->length);
	}

	if (v4l2_device->frames)
		free(v4l2_device->frames);
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

	v4l2_device->frames_num = reqbuf.count;
	v4l2_device->frames = calloc(v4l2_device->frames_num, sizeof(*(v4l2_device->frames)));
	if (v4l2_device->frames == NULL) {
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

		(v4l2_device->frames+i)->user = v4l2_device;
		(v4l2_device->frames+i)->length = buffer.length;
		(v4l2_device->frames+i)->data = mmap(NULL, buffer.length, PROT_READ | PROT_WRITE, MAP_SHARED, v4l2_device->fd, buffer.m.offset);
		if ((v4l2_device->frames+i)->data == MAP_FAILED) {
			v4l2_device_stop_capture(v4l2_device, i);
			return -1;
		}

		if (ioctl(v4l2_device->fd, VIDIOC_QBUF, &buffer) == -1) {
			v4l2_device_stop_capture(v4l2_device, i+1);
			return -1;
		}
	}

	return 0;
}

static void v4l2_device_handle_frame_defer(struct object* object) {
	struct frame* frame = frame_from_object(object);
	struct v4l2_device* v4l2_device = frame->user;
	struct v4l2_buffer buffer;

	memset (&buffer, 0, sizeof (buffer));
	buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buffer.memory = V4L2_MEMORY_MMAP;
	buffer.index = frame - v4l2_device->frames;

	ioctl(v4l2_device->fd, VIDIOC_QBUF, &buffer);
}

static void v4l2_device_handle_rx(void* data) {
	struct v4l2_device* v4l2_device = data;
	struct v4l2_buffer buffer;

	memset (&buffer, 0, sizeof (buffer));
	buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buffer.memory = V4L2_MEMORY_MMAP;

	if (ioctl(v4l2_device->fd, VIDIOC_DQBUF, &buffer) == 0) {
		init_frame(v4l2_device->frames+buffer.index, &v4l2_device_handle_frame_defer);
		frame_request_queue_process(&v4l2_device->queue, v4l2_device->frames+buffer.index);
		frame_put(v4l2_device->frames+buffer.index);
	}
}

struct v4l2_device* init_v4l2_device(const char* filename, struct event_loop* event_loop) {
	struct v4l2_device* v4l2_device = NULL;

	v4l2_device = malloc(sizeof(struct v4l2_device));
	if (v4l2_device == NULL)
		goto err;

	if ((v4l2_device->fd = open(filename, O_NONBLOCK | O_RDWR)) == -1)
		goto err_open;

	if (init_frame_request_queue(&v4l2_device->queue) == -1)
		goto err_init_frame_request_queue;

	v4l2_device->event_loop_request.event_loop   = event_loop;
	v4l2_device->event_loop_request.fd           = v4l2_device->fd;
	v4l2_device->event_loop_request.handle_event = &v4l2_device_handle_rx;
	v4l2_device->event_loop_request.user         = v4l2_device;

	v4l2_device->frames = NULL;

	if (v4l2_device_setup_capture(v4l2_device) == -1)
		goto err_setup_capture;

	if (v4l2_device_stream_on(v4l2_device) == -1)
		goto err_stream_on;

	return v4l2_device;

err_stream_on:
	v4l2_device_stop_capture(v4l2_device, 0);
err_setup_capture:
	free_frame_request_queue(&v4l2_device->queue);
err_init_frame_request_queue:
	close(v4l2_device->fd);
err_open:
	free(v4l2_device);
err:
	return NULL;
}

void free_v4l2_device(struct v4l2_device* v4l2_device) {
	v4l2_device_stop_capture(v4l2_device, 0);
	free_frame_request_queue(&v4l2_device->queue);
	close(v4l2_device->fd);
	free(v4l2_device);
}

int v4l2_device_attach_event_loop(struct v4l2_device* v4l2_device, struct event_loop* event_loop) {
	if (v4l2_device->event_loop_request.event_loop != NULL)
		return -1;

}

void v4l2_device_add_frame_request(struct v4l2_device* v4l2_device, struct frame_request* request) {
	frame_request_queue_enqueue(&v4l2_device->queue, request);
}

void v4l2_device_del_frame_request(struct v4l2_device* v4l2_device, struct frame_request* request) {
	frame_request_queue_dequeue(&v4l2_device->queue, request);
}

int v4l2_device_get_format(struct v4l2_device* v4l2_device, struct v4l2_pix_format* v4l2_pix_format) {
	struct v4l2_format format;

	memset(&format, 0, sizeof(format));
	format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (ioctl(v4l2_device->fd, VIDIOC_G_FMT, &format) == -1) {
		return -1;
	}

	*v4l2_pix_format = format.fmt.pix;

	return 0;
}
