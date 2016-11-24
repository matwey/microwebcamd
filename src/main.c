#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

#include <event_loop.h>
#include <v4l2_device.h>
#include <jpeg.h>

static void handle_complete(struct frame_request* req) {
	struct jpeg_encoder* jpeg_encoder = req->user;

	printf("complete %p %zu %p %p\n", req, req->count, req->complete, req->frame);
	frame_put(req->frame);

	if (req->count == 0) {
		req->count = 10;
		jpeg_encoder_add_frame_request(jpeg_encoder, req);
	}
}

int main(int argc, char** argv) {
	struct event_loop* event_loop;
	struct v4l2_device* v4l2_device;
	struct jpeg_encoder* jpeg_encoder;
	struct frame_request req;

	event_loop = init_event_loop();
	if (event_loop == NULL) {
		perror("init_event_loop");
		return 1;
	}

	v4l2_device = init_v4l2_device("/dev/video0", event_loop);
	if (v4l2_device == NULL) {
		perror("init_v4l2_device");
		return 1;
	}

	jpeg_encoder = init_jpeg_encoder(v4l2_device);
	if (jpeg_encoder == NULL) {
		perror("init_jpeg_encoder");
		return 1;
	}

	req.complete = &handle_complete;
	req.user = jpeg_encoder;
	req.count = 10;
	jpeg_encoder_add_frame_request(jpeg_encoder, &req);

	while (!event_loop_wait(event_loop, 1000));

	free_jpeg_encoder(jpeg_encoder);
	free_v4l2_device(v4l2_device);
	free_event_loop(event_loop);

	return 0;
}
