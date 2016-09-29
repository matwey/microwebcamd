#ifndef _FRAME_REQUEST_H
#define _FRAME_REQUEST_H

#include <list.h>

#include <pthread.h>

struct frame_request {
	struct list_head list;
	void (*complete)(struct frame_request*);
	void* data;
	size_t count;
};

struct frame_request_queue {
	pthread_spinlock_t lock;
	struct list_head queue;
};

struct frame_request_queue* init_frame_request_queue();
void free_frame_request_queue(struct frame_request_queue* queue);

static inline void frame_request_queue_enqueue(struct frame_request_queue* queue, struct frame_request* request) {
	pthread_spin_lock(&queue->lock);
	list_head_push_back(&queue->queue, &request->list);
	pthread_spin_unlock(&queue->lock);
}

static inline void frame_request_queue_dequeue(struct frame_request_queue* queue, struct frame_request* request) {
	pthread_spin_lock(&queue->lock);
	list_head_remove(&request->list);
	pthread_spin_unlock(&queue->lock);
}

static inline int frame_request_queue_empty(struct frame_request_queue* queue) {
	int ret;

	pthread_spin_lock(&queue->lock);
	ret = list_head_empty(&queue->queue);
	pthread_spin_unlock(&queue->lock);

	return ret;
}

#endif // _FRAME_REQUEST_H
