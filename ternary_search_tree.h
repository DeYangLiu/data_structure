#ifndef _TERNARY_SEARCH_TREE_H_
#define _TERNARY_SEARCH_TREE_H_
typedef unsigned short node_key_t;

struct TraverseCtx;
struct Node;


typedef void (*TraverseFn)(struct TraverseCtx *ctx, int curPathLen, void *val);

typedef void (*FreeFn)(void *val);

struct TraverseCtx{
	TraverseFn cb;
	node_key_t* path;
	int pathLen;
	
	void *userData;
};


void tree_insert(struct Node** root, node_key_t *keys, int num, void *val);
void* tree_search(struct Node *root, node_key_t *keys, int num);
int tree_delete(struct Node **root, node_key_t *keys, int num, int flag, FreeFn fn);
int tree_traverse(struct Node* root, node_key_t *keys, int num, 
		struct TraverseCtx *ctx);

struct Node *tree_nil(void);

typedef int (*CompareFn)(void *from, void *to);

struct CompareCtx{
	FreeFn freeFn;
	CompareFn cmpFn;
	void *cmpTo;
};
int tree_compare_and_set(struct Node *root, node_key_t *keys, int num, struct CompareCtx *ctx);


#endif
