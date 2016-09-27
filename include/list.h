#ifndef _LIST_H
#define _LIST_H

struct list_head {
	struct list_head* next;
	struct list_head* prev;
};

static inline void init_list_head(struct list_head* head) {
	head->next = head;
	head->prev = head;
}

static inline int list_empty(const struct list_head *head) {
	return head == head->next;
}

#endif // _LIST_H
