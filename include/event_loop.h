#ifndef _EVENT_LOOP
#define _EVENT_LOOP

struct event_loop;
struct event_loop_request;

struct event_loop* init_event_loop();
void free_event_loop(struct event_loop* event_loop);

struct event_loop_request* event_loop_add_request(struct event_loop* event_loop, int fd, void (*handle_rx)(void*), void* data);
void event_loop_del_request(struct event_loop_request* request);
int event_loop_wait(struct event_loop* event_loop, int timeout);

#endif // _EVENT_LOOP
