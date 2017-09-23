/* red black tree test.
gcc rb-test.c -Drb_test=main -g -O0 && ./a.out

ref
 https://github.com/glk/critbit
*/
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>

#include <stdint.h>
#include <stdbool.h>
//#define NDEBUG
#include <assert.h>
#define RB_COMPACT
#include "rb.h"

typedef struct ex_node_s ex_node_t;
struct ex_node_s {
	int key;
	rb_node(ex_node_t) ex_link;
};

static int ex_cmp(ex_node_t *a, ex_node_t *b)
{
	return a->key - b->key;
}

typedef rbt(ex_node_t) ex_t;
rb_gen(static, ex_, ex_t, ex_node_t, ex_link, ex_cmp);

static ex_node_t* func_cb(ex_t *tree, ex_node_t *node, void *sum)
{
	*(int*)sum += node->key;
	return NULL;
}

static void test_rb_function(void)
{
	int maxCnt = 101;
	ex_t tree;
	ex_node_t find, *found, *el, *xel;
	int sum;
	
	xel = malloc(sizeof(*el) * maxCnt);
	ex_new(&tree);
	
	int i;
	for(i = 0; i < maxCnt; ++i){
		el = &xel[i];
		el->key = i;
		ex_insert(&tree, el);
	}
	sum = 0;
	ex_iter(&tree, NULL, func_cb, &sum);
	printf("sum %d\n", sum);
	
	find.key = maxCnt/2;
	found = ex_search(&tree, &find);
	ex_remove(&tree, found);

	sum = 0;
	ex_iter(&tree, NULL, func_cb, &sum);
	printf("sum %d\n", sum);

	free(xel);
}

static void test_rb_benchmark(int sLoopCount)
{
	ex_t tree;
	ex_node_t find, *found, *el, *xel;
	xel = malloc(sizeof(*el) * sLoopCount);

	struct timeval tmStart, tmEnd;	
	gettimeofday(&tmStart, NULL);

	int i,j;
	for(i = 2; i < sLoopCount; ++i){
		ex_new(&tree);
		for(j = 0; j < i; ++j){
			el = &xel[j];
			el->key = j;
			ex_insert(&tree, el);
		}

		for( j = 0; j < i; ++j){
			find.key = j;
			if(!(found = ex_search(&tree, &find)))assert(1==0);
		}
		ex_remove(&tree, found);

		for( j = 0; j < i-1; ++j){
			find.key = j;
			if(!(found = ex_search(&tree, &find)))assert(2==0);
		}
		
	}
	gettimeofday(&tmEnd, NULL);
	
	free(xel);

	double t;
	t = tmEnd.tv_sec - tmStart.tv_sec;
	t += (tmEnd.tv_usec - tmStart.tv_usec)/(double)1000000;
	printf("%d iterations in %lf seconds; %lf iterations/s\n",
		   sLoopCount, t, sLoopCount/t);
}

int rb_test(int ac, char **av)
{
	test_rb_function();
	test_rb_benchmark(1000);

	return 0;
}
