#ifndef _JPEG_H
#define _JPEG_H

#include <v4l2_device.h>
#include <frame_request.h>

struct jpeg_encoder;

struct jpeg_encoder* init_jpeg_encoder(struct v4l2_device* v4l2_device);
void free_jpeg_encoder(struct jpeg_encoder* jpeg_encoder);

void jpeg_encoder_add_frame_request(struct jpeg_encoder* jpeg_encoder, struct frame_request* request);
void jpeg_encoder_del_frame_request(struct jpeg_encoder* jpeg_encoder, struct frame_request* request);

#endif // _JPEG_H
