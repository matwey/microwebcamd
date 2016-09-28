#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

#include <event_loop.h>

int main(int argc, char** argv) {
	struct event_loop* event_loop;
	event_loop = init_event_loop();
	if (event_loop == NULL) {
		perror("init_event_loop");
		return 1;
	}

	while (!event_loop_wait(event_loop, 1000));

	free_event_loop(event_loop);

	return 0;
}
