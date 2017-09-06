#include <sys/defs.h>
#include <stdio.h>

struct node {
	struct node *next;
	size_t size;
};
typedef struct node Node;

Node head;
Node *head_free_list = NULL;

void *malloc(size_t size) {
	if(head_free_list == NULL) {
		head.size=0;
		head_free_list = &head;
	}
	Node *prev = head_free_list;
	Node *p = head_free_list->next;

	while(p) {
		if(p->size == size) {
			prev->next = p->next;
			break;

		}
		else if(p->size>size) {
			p->size -=size;
			p = p + p->size;
			break;

		}
		if (p == head_free_list) {
			//get more space
		}
		prev = p;
		p = p->next;
	}
	head_free_list = prev;
	putchar('c');
	return (void*) p;
}