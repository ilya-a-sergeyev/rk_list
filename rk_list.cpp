#include <sys/types.h>
#include <string.h>
#include "rk_list.h"

struct rk_list_node {
	struct rk_list_node *prev;	/**< Previous element                    */
	struct rk_list_node *next;	/**< Next element                        */
	struct rk_list *list;		/**< Parent list (NULL if not linked-in) */
	void *data;			/**< User-data                           */
};

rk_list::rk_list()
{
	count = 0;
	destroy = NULL;
	head = NULL;
}

rk_list::rk_list(node_destructor func)
{
	count = 0;
	destroy = func;
	head = NULL;
}

rk_list::rk_list(const rk_list &other)
{
// TODO - copy contain of the list
	destroy = other.destroy;
	head = other.head;
	count = other.count;
}

rk_list & rk_list::operator=(const rk_list &other)
{
// TODO - copy contain of the list
	destroy = other.destroy;
	head = other.head;
	count = other.count;
	return *this;
}

rk_list::~rk_list()
{
	flush();
}

int32_t rk_list::list_is_empty()
{
	return count?0:1;
}

int32_t rk_list::list_size()
{
	return count;
}

int32_t rk_list::flush()
{
	while (count)
		del_at_tail(0, 0);
	return 0;
}

int32_t rk_list::add_at_head(void *data, int32_t size)
{
	pthread_mutex_lock(&mutex);
	rk_list_node *nn = new rk_list_node();
	nn->prev = NULL;
	nn->next = head;
	head->prev = nn;
	nn->list = this;
	nn->data = data;
	count++;
	head = nn;
	pthread_mutex_unlock(&mutex);
	return count;
}

int32_t rk_list::add_at_tail(void *data, int32_t size)
{
	pthread_mutex_lock(&mutex);
	rk_list_node *nn = new rk_list_node();
	rk_list_node *tail = head;
	if (tail) {
		while (tail->next)
			tail = tail->next;
		nn->prev = tail;
		tail->next = nn;
	}
	else {
		nn->prev = NULL;
		head = nn;
	}
	nn->next = NULL;
	nn->list = this;
	nn->data = data;
	count++;
	pthread_mutex_unlock(&mutex);
	return count;
}

int32_t rk_list::del_at_head(void *data, int32_t size)
{
	pthread_mutex_lock(&mutex);
	if (head) {
		rk_list_node *t_head = head;
		head = head->next;
		head->prev = NULL;
		count--;
		if (data)
			memcpy(data, t_head->data, size);
		(destroy)(t_head->data);
		delete t_head;
	}
	pthread_mutex_unlock(&mutex);
	return count;
}

int32_t rk_list::del_at_tail(void *data, int32_t size)
{
	pthread_mutex_lock(&mutex);
	rk_list_node *tail = head;
	while (tail->next)
		tail = tail->next;
	if (tail->prev) {
		tail->prev->next = NULL;
		tail->prev = NULL;
		count--;
		if (data)
			memcpy(data, tail->data, size);
		(destroy)(tail->data);
		delete(tail);
	}
	pthread_mutex_unlock(&mutex);
	return count;
}
