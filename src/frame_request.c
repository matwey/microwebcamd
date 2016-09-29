#include <frame_request.h>

#include <stdlib.h>
#include <pthread.h>

struct frame_request_queue* init_frame_request_queue() {
	struct frame_request_queue* queue = NULL;

	queue = malloc(sizeof(struct frame_request_queue));
	if (queue == NULL)
		return NULL;

	if (pthread_spin_init(&queue->lock, PTHREAD_PROCESS_PRIVATE) != 0) {
		free(queue);
		return NULL;
	}

	init_list_head(&queue->queue);

	return queue;
}

void free_frame_request_queue(struct frame_request_queue* queue) {
	pthread_spin_destroy(&queue->lock);
	free(queue);
}
