#ifndef _LIST_H
#define _LIST_H

#include <stddef.h>

struct list_head {
	struct list_head* next;
	struct list_head* prev;
};

static inline void init_list_head(struct list_head* head) {
	head->next = head;
	head->prev = head;
}

static inline int list_head_empty(const struct list_head* head) {
	return head == head->next;
}

static inline void list_head_insert(struct list_head* head, struct list_head* item) {
	item->prev = head;
	item->next = head->next;
	head->next->prev = item;
	head->next = item;
}

static inline void list_head_push_back(struct list_head *head, struct list_head *item) {
	list_head_insert(head->prev, item);
}

static inline void list_head_remove(struct list_head *item) {
	item->next->prev = item->prev;
	item->prev->next = item->next;
}

#define list_for_each(pos, head) for (pos = (head)->next; pos != (head); pos = pos->next)

#define list_entry(ptr, type, member) ((type*)((char*)(ptr) - offsetof(type, member)))

#define list_for_each_entry(pos, head, type, member) \
	for (pos = list_entry((head)->next, type, member); \
	     &pos->member != (head); \
	     pos = list_entry(pos->member.next, type, member))

#endif // _LIST_H
