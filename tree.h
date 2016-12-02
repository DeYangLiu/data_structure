#ifndef _TREE_H_
#define _TREE_H_

#include "compact.h"

struct Node {
	int key;
	void *val;
	struct Node *sibling;
	struct Node *child;
};

typedef void (*TraverseFn)(void* ctx, void *val);
typedef void (*NodeFn)(void *val);

struct Node *tree_newNode(int key, void *val, struct Node *child, struct Node *sibling );
void tree_freeNode(struct Node *node);

struct Node* tree_find(struct Node *root, U16 keys[], int num);
//delete k1..kn.*
int tree_insert(struct Node *root, U16 keys[], int num, void *val);
int tree_delete(struct Node *root, U16 keys[], int num, NodeFn freeVal);

void tree_traverse(struct Node *root, TraverseFn fn, void *ctx);

#endif
