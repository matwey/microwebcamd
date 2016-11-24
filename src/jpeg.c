#include <jpeg.h>
#include <frame_request.h>
#include <frame.h>

struct jpeg_encoder {
	struct v4l2_device* v4l2_device;
	struct frame_request v4l2_request;
	struct frame_request_queue queue;
	struct v4l2_pix_format v4l2_pix_format;
};

static void jpeg_encoder_handle_frame_defer(struct object* object) {
	struct frame* frame = frame_from_object(object);

	free(frame);
}

static int jpeg_encoder_do_encode(struct jpeg_encoder* jpeg_encoder, struct frame* source, struct frame* dest) {
	struct v4l2_pix_format* v4l2_pix_format = &jpeg_encoder->v4l2_pix_format;

	printf("SRC %d x %d fmt %d\n", v4l2_pix_format->width, v4l2_pix_format->height, v4l2_pix_format->pixelformat);
}

static void jpeg_encoder_handle_v4l2_complete(struct frame_request* req) {
	struct jpeg_encoder* jpeg_encoder = req->user;
	struct frame* frame = NULL;

	frame = malloc(sizeof(struct frame));
	if (frame == NULL)
		return;

	init_frame(frame, &jpeg_encoder_handle_frame_defer);
	jpeg_encoder_do_encode(jpeg_encoder, req->frame, frame);
	frame_request_queue_process(&jpeg_encoder->queue, frame);
	frame_put(frame);

	frame_put(req->frame);

	if (req->count == 0) {
		req->count = 10;
		v4l2_device_add_frame_request(jpeg_encoder->v4l2_device, req);
	}
}

struct jpeg_encoder* init_jpeg_encoder(struct v4l2_device* v4l2_device) {
	struct jpeg_encoder* jpeg_encoder = NULL;

	jpeg_encoder = malloc(sizeof(struct jpeg_encoder));
	if (jpeg_encoder == NULL)
		return NULL;

	if (init_frame_request_queue(&jpeg_encoder->queue) == -1) {
		free(jpeg_encoder);
		return NULL;
	}

	jpeg_encoder->v4l2_device = v4l2_device;
	jpeg_encoder->v4l2_request.complete = &jpeg_encoder_handle_v4l2_complete;
	jpeg_encoder->v4l2_request.user = jpeg_encoder;
	jpeg_encoder->v4l2_request.count = 10;

	if (v4l2_device_get_format(jpeg_encoder->v4l2_device, &jpeg_encoder->v4l2_pix_format) == -1) {
		free(jpeg_encoder);
		return NULL;
	}

	v4l2_device_add_frame_request(jpeg_encoder->v4l2_device, &jpeg_encoder->v4l2_request);

	return jpeg_encoder;
}
void free_jpeg_encoder(struct jpeg_encoder* jpeg_encoder) {
	v4l2_device_del_frame_request(jpeg_encoder->v4l2_device, &jpeg_encoder->v4l2_request);
	free_frame_request_queue(&jpeg_encoder->queue);
	free(jpeg_encoder);
}
void jpeg_encoder_add_frame_request(struct jpeg_encoder* jpeg_encoder, struct frame_request* request) {
	frame_request_queue_enqueue(&jpeg_encoder->queue, request);
}
void jpeg_encoder_del_frame_request(struct jpeg_encoder* jpeg_encoder, struct frame_request* request) {
	frame_request_queue_dequeue(&jpeg_encoder->queue, request);
}
