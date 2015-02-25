#include <assert.h>
// #include <error.h>
#include <stdio.h>
#include <stdlib.h>
#include "queue.h"
#include <pthread.h>

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
int s = 0;

void queue_init(queue_t *q) {
	node_t *new = malloc(sizeof(node_t));

	if (new == NULL) {
		perror("malloc failed");
		exit(EXIT_FAILURE);
	}

	new->next = NULL;
	q->head = q->tail = new;
}

void queue_put(queue_t *q, void *item) {
	printf("put");
	s = pthread_mutex_lock(&mtx);
	if (s != 0) exit(s);

	node_t *new = malloc(sizeof(node_t));
	if (new == NULL) {
		perror("malloc failed");
		exit(EXIT_FAILURE);
	}

	new->item = item;
	new->next = NULL;

	/* add the new node to the tail */
	q->tail->next = new;
	q->tail = new;
	s = pthread_mutex_unlock(&mtx);
	if (s != 0) exit(s);
}

void *queue_get(queue_t *q) {
	s = pthread_mutex_lock(&mtx);
	if (s != 0) exit(s);

	node_t *old = q->head;

	/* note that the head contains a 'dummy' node. That's why we test
	 * old->next. */
	if (old->next == NULL) {
		return NULL; /* queue was empty */
	}

	void *item = old->next->item;

	/* update the head and free the old memory */
	q->head = old->next;
	free(old);
	s = pthread_mutex_unlock(&mtx);
	if (s != 0) exit(s);

	return item;
}

int main(int argc, char **argv) {
	queue_t q;
	queue_init(&q);
	int val = 42;

	queue_put(&q, &val);
	queue_put(&q, &val);

	assert(42 == *(int *)queue_get(&q));
	assert(42 == *(int *)queue_get(&q));
	assert(NULL == queue_get(&q));
	assert(NULL == queue_get(&q));

	return 0;
}
