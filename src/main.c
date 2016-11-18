#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

#include <event_loop.h>
#include <v4l2_device.h>

static void handle_complete(struct frame_request* req) {
	struct v4l2_device* v4l2_device = req->data;

	printf("complete %p %zu %p\n", req, req->count, req->complete);
	if (req->count == 0) {
		req->count = 10;
		v4l2_device_add_frame_request(v4l2_device, req);
	}
}

int main(int argc, char** argv) {
	struct event_loop* event_loop;
	struct v4l2_device* v4l2_device;
	struct frame_request req;

	event_loop = init_event_loop();
	if (event_loop == NULL) {
		perror("init_event_loop");
		return 1;
	}

	v4l2_device = init_v4l2_device("/dev/video0");
	if (v4l2_device == NULL) {
		perror("init_v4l2_device");
		return 1;
	}

	if (v4l2_device_attach_event_loop(v4l2_device, event_loop) == -1) {
		perror("v4l2_device_attach_event_loop");
		return 1;
	}

	req.complete = &handle_complete;
	req.data = v4l2_device;
	req.count = 10;
	v4l2_device_add_frame_request(v4l2_device, &req);

	while (!event_loop_wait(event_loop, 1000));

	free_v4l2_device(v4l2_device);
	free_event_loop(event_loop);

	return 0;
}
