#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
	int i;
	for(i = 0; i < NR_WP; i ++) {
		wp_pool[i].NO = i;
		wp_pool[i].next = &wp_pool[i + 1];
	}
	wp_pool[NR_WP - 1].next = NULL;

	head = NULL;
	free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP* new_wp() {
	if(free_ == NULL) {
		printf("Free is null\n");
		assert(0);
	}
	WP *f,*h;
	f = free_;
	free_ = free_->next;
	f->next = NULL;
	h = head;
	if(h == NULL) {
		head = f;
		h = head;
	}
	else {
		while(h != NULL) 
			h = h->next;
		h->next = f;
	}
	return f;
}

void free_wp(WP *wp) {
	WP *f,*h;
	f = free_;
	if(f == NULL) {
		free_ = wp;
		f = free_;
	}
	else {
		while(f != NULL)
			f = f->next;
		f->next = wp;
	}
	h = head;
	if(head == NULL) {
		printf("Head is null\n");
		assert(0);
	}
	if(head->NO == wp->NO) {
		head = head->next;
	}
	else {
		while(h->next != NULL && h->next->NO != wp->NO)
			h = h->next;
		if(h->next->NO == wp->NO)
			h->next = h->next->next;
		else 
			assert(0);
	}
	wp->next = NULL;
	wp->tag = 0;
	wp->val = 0;
	wp->expr[0] = '\0';
}
