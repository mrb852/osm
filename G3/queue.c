#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "queue.h"
#include <pthread.h>

static pthread_mutex_t mtx;
int s = 0;

void queue_init(queue_t *q) {
  pthread_mutex_init(&mtx, NULL);
  node_t *new = malloc(sizeof(node_t));

  if (new == NULL) {
    perror("malloc failed");
    exit(EXIT_FAILURE);
  }

  new->next = NULL;
  q->head = q->tail = new;
}

void queue_put(queue_t *q, void *item) {

  pthread_mutex_lock(&mtx);

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
  pthread_mutex_unlock(&mtx);

}

void *queue_get(queue_t *q) {

  pthread_mutex_lock(&mtx);

  node_t *old = q->head;

  /* note that the head contains a 'dummy' node. That's why we test
   * old->next. */
  if (old->next == NULL) {
  	pthread_mutex_unlock(&mtx);
    return NULL; /* queue was empty */
  }

  void *item = old->next->item;

  /* update the head and free the old memory */
  q->head = old->next;
  free(old);

  pthread_mutex_unlock(&mtx);

  return item;
}

typedef struct put_args {
	queue_t* queue;
	int val;
} put_args;


static void* put(void* args) {
	put_args *pargs = args;
	queue_put(pargs->queue, &pargs->val);
	pthread_exit(NULL);
	return NULL;
}

static void* get(void* args) {
	put_args *pargs = args;
	queue_get(pargs->queue);
	pthread_exit(NULL);
}

int main(int argc, char **argv) {

	long MAX = 10000;
	pthread_t put_threads[MAX];
	put_args args[MAX];
	pthread_t get_threads[MAX];
	queue_t q;
	queue_init(&q);

	printf("putting elems\n");
	for (int i = 0; i < MAX; i++) {
		put_args arg = {.queue = &q, .val = i};
		args[i] = arg;
		pthread_create(&put_threads[i], NULL, put, &args[i]);
	}

	for (int i = 0; i < MAX; i++) {
		pthread_join(put_threads[i], NULL);
	}

	printf("getting elems\n");
	for (int i = 0; i < MAX; i++) {
		pthread_create(&get_threads[i], NULL, get, &args[i]);
	}

	for (int i = 0; i < MAX; i++) {
		pthread_join(get_threads[i], NULL);
	}

	assert(NULL == queue_get(&q));
	printf("successfully inserted and popped all elems\n");
	return 0;
}
