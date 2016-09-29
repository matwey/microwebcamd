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

static inline int list_head_empty(const struct list_head* head) {
	return head == head->next;
}

static inline void list_head_insert(struct list_head* head, struct list_head* item) {
	item->prev = head;
	item->next = head->next;
	head->next = item;
}

static inline void list_head_push_back(struct list_head *head, struct list_head *item) {
	list_head_insert(head->prev, item);
}

static inline void list_head_remove(struct list_head *item) {
	item->next->prev = item->prev;
	item->prev->next = item->next;
}

#endif // _LIST_H
