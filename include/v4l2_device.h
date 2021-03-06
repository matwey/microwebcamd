#ifndef _V4L2_DEVICE_H
#define _V4L2_DEVICE_H

#include <event_loop.h>
#include <frame_request.h>

struct v4l2_device;

struct v4l2_device* init_v4l2_device(const char* filename, struct event_loop* event_loop);
void free_v4l2_device(struct v4l2_device* v4l2_device);
void v4l2_device_add_frame_request(struct v4l2_device* v4l2_device, struct frame_request* request);
void v4l2_device_del_frame_request(struct v4l2_device* v4l2_device, struct frame_request* request);
int v4l2_device_get_format(struct v4l2_device* v4l2_device, struct v4l2_pix_format* v4l2_pix_format);

#endif // _V4L2_DEVICE_H
