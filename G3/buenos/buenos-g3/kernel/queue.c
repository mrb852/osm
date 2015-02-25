#include <assert.h>
#include <error.h>
#include <stdio.h>
#include <stdlib.h>

#include "queue.h"

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
}

void *queue_get(queue_t *q) {
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
