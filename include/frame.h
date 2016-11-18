#ifndef _FRAME_H
#define _FRAME_H

#include <object.h>

struct frame {
	struct object object;
	void* data;
	size_t length;
//	struct v4l2_pix_format v4l2_pix_format;
	void* user;
};

static inline struct frame* frame_from_object(struct object* object) {
	return (struct frame*)((void*)object - offsetof(struct frame, object));
}

static inline void init_frame(struct frame* frame, void (*defer)(struct object*)) {
	init_object(&frame->object, defer);
}

static inline struct frame* frame_get(struct frame* frame) {
	return object_get(&frame->object) ? frame : NULL;
}

static inline void frame_put(struct frame* frame) {
	object_put(&frame->object);
}

#endif // _FRAME_H
