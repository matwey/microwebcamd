#ifndef _OBJECT_H
#define _OBJECT_H

#include <atomic_ops.h>

struct object {
	AO_t ref_count;
	void (*defer)(struct object*);
};

static inline void init_object(struct object* obj, void (*defer)(struct object*)) {
	obj->defer = defer;
	AO_store_full(&obj->ref_count, 1);
}

static inline int object_get(struct object* obj) {
	AO_t ref_count;

	while ((ref_count = AO_load(&obj->ref_count))) {
		if (AO_compare_and_swap(&obj->ref_count, ref_count, ref_count+1))
			return 1;
	}

	return 0;
}

static inline void object_put(struct object* obj) {
	if (AO_fetch_and_sub1(&obj->ref_count) == 2 && obj->defer)
		obj->defer(obj);
}

#endif // _OBJECT_H
