#ifndef _FRAME_REQUEST_H
#define _FRAME_REQUEST_H

#include <list.h>
#include <frame.h>

#include <pthread.h>

struct frame_request {
	struct list_head list;
	void (*complete)(struct frame_request*);
	struct frame* frame;
	void* user;
	size_t count;
};

struct frame_request_queue {
	pthread_spinlock_t lock;
	pthread_spinlock_t lock_next;
	struct list_head queue;
	struct list_head queue_next;
};

int init_frame_request_queue(struct frame_request_queue* queue);
void free_frame_request_queue(struct frame_request_queue* queue);

static inline void frame_request_queue_enqueue(struct frame_request_queue* queue, struct frame_request* request) {
	pthread_spin_lock(&queue->lock_next);
	list_head_push_back(&queue->queue_next, &request->list);
	pthread_spin_unlock(&queue->lock_next);
}

static inline void frame_request_queue_dequeue(struct frame_request_queue* queue, struct frame_request* request) {
	pthread_spin_lock(&queue->lock);
	list_head_remove(&request->list);
	pthread_spin_unlock(&queue->lock);
}

static inline void frame_request_queue_process(struct frame_request_queue* queue, struct frame* frame) {
	struct frame_request* pos;
	struct frame_request* next;

	pthread_spin_lock(&queue->lock);

	pthread_spin_lock(&queue->lock_next);
	list_head_splice(&queue->queue, &queue->queue_next);
	pthread_spin_unlock(&queue->lock_next);

	list_for_each_entry_safe(pos, next, &queue->queue, struct frame_request, list) {
		if (--(pos->count) == 0) {
			list_head_remove(&pos->list);
		}
		pos->frame = frame_get(frame);
		pos->complete(pos);
	}
	pthread_spin_unlock(&queue->lock);
}

#endif // _FRAME_REQUEST_H
