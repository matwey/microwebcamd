#include <jpeg.h>
#include <frame_request.h>
#include <frame.h>

struct jpeg_encoder {
	struct v4l2_device* v4l2_device;
	struct frame_request v4l2_request;
	struct frame_request_queue queue;
};

static void jpeg_encoder_handle_frame_defer(struct object* object) {
	struct frame* frame = frame_from_object(object);

	free(frame);
}

static void jpeg_encoder_handle_v4l2_complete(struct frame_request* req) {
	struct jpeg_encoder* jpeg_encoder = req->user;
	struct frame* frame = NULL;

	frame = malloc(sizeof(struct frame));
	if (frame == NULL)
		return;

	init_frame(frame, &jpeg_encoder_handle_frame_defer);
	frame_request_queue_process(&jpeg_encoder->queue, frame);
	frame_put(frame);

	frame_put(req->frame);
}

struct jpeg_encoder* init_jpeg_encoder(struct v4l2_device* v4l2_device) {
	struct jpeg_encoder* jpeg_encoder = NULL;

	jpeg_encoder = malloc(sizeof(struct jpeg_encoder));
	if (jpeg_encoder == NULL)
		return NULL;

	jpeg_encoder->v4l2_device = v4l2_device;
	jpeg_encoder->v4l2_request.complete = &jpeg_encoder_handle_v4l2_complete;
	jpeg_encoder->v4l2_request.user = jpeg_encoder;
	jpeg_encoder->v4l2_request.count = 1;

	return jpeg_encoder;
}
void free_jpeg_encoder(struct jpeg_encoder* jpeg_encoder) {
	free(jpeg_encoder);
}
void jpeg_encoder_add_frame_request(struct jpeg_encoder* jpeg_encoder, struct frame_request* request) {
	frame_request_queue_enqueue(&jpeg_encoder->queue, request);
}
void jpeg_encoder_del_frame_request(struct jpeg_encoder* jpeg_encoder, struct frame_request* request) {
	frame_request_queue_dequeue(&jpeg_encoder->queue, request);
}
