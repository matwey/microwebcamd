#include <frame_request.h>

#include <stdlib.h>
#include <pthread.h>

int init_frame_request_queue(struct frame_request_queue* queue) {
	if (pthread_mutex_init(&queue->lock, PTHREAD_PROCESS_PRIVATE) != 0) {
		return -1;
	}
	if (pthread_mutex_init(&queue->lock_next, PTHREAD_PROCESS_PRIVATE) != 0) {
		pthread_mutex_destroy(&queue->lock);
		return -1;
	}

	init_list_head(&queue->queue);
	init_list_head(&queue->queue_next);

	return 0;
}

void free_frame_request_queue(struct frame_request_queue* queue) {
	pthread_mutex_destroy(&queue->lock_next);
	pthread_mutex_destroy(&queue->lock);
}
