#include <check.h>

#include <list.h>

START_TEST (test_list_init1) {
	struct list_head head;
	init_list_head(&head);
	ck_assert(head.next == &head);
	ck_assert(head.prev == &head);
	ck_assert(list_head_empty(&head));
}
END_TEST

START_TEST (test_list_push_back1) {
	struct list_head head;
	struct list_head item1;
	init_list_head(&head);
	list_head_push_back(&head, &item1);
	ck_assert(head.next == &item1);
	ck_assert(head.prev == &item1);
	ck_assert(item1.prev == &head);
	ck_assert(item1.next == &head);
	ck_assert(!list_head_empty(&head));
}
END_TEST

START_TEST (test_list_push_back2) {
	struct list_head head;
	struct list_head item1;
	struct list_head item2;
	init_list_head(&head);
	list_head_push_back(&head, &item1);
	list_head_push_back(&head, &item2);
	ck_assert(head.next == &item1);
	ck_assert(head.prev == &item2);
	ck_assert(item1.prev == &head);
	ck_assert(item1.next == &item2);
	ck_assert(item2.prev == &item1);
	ck_assert(item2.next == &head);
	ck_assert(!list_head_empty(&head));
}
END_TEST

START_TEST (test_list_insert1) {
	struct list_head head;
	struct list_head item1;
	struct list_head item2;
	init_list_head(&head);
	list_head_push_back(&head, &item2);
	list_head_insert(&head, &item1);
	ck_assert(head.next == &item1);
	ck_assert(head.prev == &item2);
	ck_assert(item1.prev == &head);
	ck_assert(item1.next == &item2);
	ck_assert(item2.prev == &item1);
	ck_assert(item2.next == &head);
	ck_assert(!list_head_empty(&head));
}
END_TEST

START_TEST (test_list_remove1) {
	struct list_head head;
	struct list_head item1;
	init_list_head(&head);
	list_head_push_back(&head, &item1);
	list_head_remove(&item1);
	ck_assert(head.next == &head);
	ck_assert(head.prev == &head);
	ck_assert(list_head_empty(&head));
}
END_TEST

START_TEST (test_list_for_each1) {
	struct list_head head;
	struct list_head item1;
	struct list_head item2;
	init_list_head(&head);
	list_head_push_back(&head, &item1);
	list_head_push_back(&head, &item2);
	struct list_head* it;
	int i=0;
	list_for_each(it, &head) {
		i++;
	}
	ck_assert_int_eq(i, 2);
}
END_TEST

START_TEST (test_list_item1) {
	struct item {
		int payload;
		struct list_head list;
	};
	struct item e1;
	e1.payload = 42;
	struct item* ptr = list_entry(&e1.list, struct item, list);
	ck_assert_ptr_eq(ptr, &e1);
	ck_assert_int_eq(ptr->payload, 42);
}
END_TEST

START_TEST (test_list_for_each2) {
	struct item {
		int payload;
		struct list_head list;
	};
	struct list_head head;
	struct item item1;
	struct item item2;
	init_list_head(&head);
	item1.payload = 0;
	item2.payload = 1;
	list_head_push_back(&head, &item1.list);
	list_head_push_back(&head, &item2.list);
	struct item* it;
	int i=0;
	list_for_each_entry(it, &head, struct item, list) {
		ck_assert_int_eq(i, it->payload);
		i++;
	}
	ck_assert_int_eq(i, 2);
}
END_TEST

Suite* list_suite(void) {
	Suite *s;
	TCase *tc_core;

	s = suite_create("list");

	tc_core = tcase_create("Core");

	tcase_add_test(tc_core, test_list_init1);
	tcase_add_test(tc_core, test_list_push_back1);
	tcase_add_test(tc_core, test_list_push_back2);
	tcase_add_test(tc_core, test_list_insert1);
	tcase_add_test(tc_core, test_list_remove1);
	tcase_add_test(tc_core, test_list_for_each1);
	tcase_add_test(tc_core, test_list_for_each2);
	tcase_add_test(tc_core, test_list_item1);

	suite_add_tcase(s, tc_core);

	return s;
}

int main(void) {
	int number_failed;
	Suite *s;
	SRunner *sr;

	s = list_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	return (number_failed == 0) ? 0 : 1;
}

