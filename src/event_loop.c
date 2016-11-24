#include <event_loop.h>

#include <stdlib.h>
#include <sys/epoll.h>
#include <unistd.h>

#define MAX_EVENTS 10

struct event_loop {
	int fd;
};

struct event_loop* init_event_loop() {
	struct event_loop* event_loop = NULL;
	event_loop = malloc(sizeof(struct event_loop));
	if (event_loop == NULL)
		return NULL;

	event_loop->fd = epoll_create1(0);
	if (event_loop->fd == -1) {
		free(event_loop);
		return NULL;
	}

	return event_loop;
}

void free_event_loop(struct event_loop* event_loop) {
	close(event_loop->fd);
	free(event_loop);
}

int event_loop_add_request(struct event_loop* event_loop, struct event_loop_request* request) {
	struct epoll_event ev;

	ev.events = EPOLLIN;
	ev.data.ptr = request;

	if (epoll_ctl(event_loop->fd, EPOLL_CTL_ADD, request->fd, &ev) == -1)
		return -1;

	request->event_loop = event_loop;

	return 0;
}

void event_loop_del_request(struct event_loop_request* request) {
	struct epoll_event ev;
	struct event_loop* event_loop = request->event_loop;

	epoll_ctl(event_loop->fd, EPOLL_CTL_DEL, request->fd, &ev);
}

int event_loop_wait(struct event_loop* event_loop, int timeout) {
	struct epoll_event events[MAX_EVENTS];
	int i, nfds = MAX_EVENTS;

	do {
		nfds = epoll_wait(event_loop->fd, events, MAX_EVENTS, timeout);
		if (nfds == -1)
			return -1;

		for (i = 0; i < nfds; ++i) {
			struct event_loop_request* req = events[i].data.ptr;
			req->handle_event(req->user);
		}
	} while (nfds == MAX_EVENTS);

	return 0;
}
