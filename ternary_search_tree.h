#ifndef _TERNARY_SEARCH_TREE_H_
#define _TERNARY_SEARCH_TREE_H_
typedef unsigned short node_key_t;

typedef void (*TraverseFn)(node_key_t *path, int len, void *val);

typedef void (*FreeFn)(void *val);

struct Node;

void insert(struct Node** root, node_key_t *keys, int num, void *val);
void* search(struct Node *root, node_key_t *keys, int num);
int delete(struct Node **root, node_key_t *keys, int num, FreeFn fn);
void traverse(struct Node* root, TraverseFn fn, node_key_t* path, int depth);


#endif
