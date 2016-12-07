#include <stdlib.h>
#include <string.h>
#include "ternary_search_tree.h"
#include "compact.h"

#define TST_DBG(...) LOG(__VA_ARGS__)

#define CONFIG_REBALANCE_TREE 1

//this can half the number of mallocs.
#define USE_GROUP_ALLOC 1

struct Node
{
	void *val;
    node_key_t key;
	
    char isEnd;
	#if CONFIG_REBALANCE_TREE
	char level;
	#endif
	
    struct Node *left, *eq, *right;
};

#if CONFIG_REBALANCE_TREE
static struct Node s_nil = {.left = &s_nil, .right = &s_nil};

struct Node *tree_nil(void)
{
	return &s_nil;
}

static void skew(struct Node **a)
{
    if ((*a)->level != 0) {
		//remove left horizontal links
        if ((*a)->left->level == (*a)->level) {
			TST_DBG("at %x->%x:%d\n", (*a)->key, (*a)->left->key, (*a)->level);
			//right rotate
			struct Node *b = (*a)->left;
			(*a)->left = b->right;
			b->right = *a;
			*a = b;
        }
    }
}

static void split(struct Node **a)
{
	//remove consecutive right horizontal links.
	if((*a)->level != 0 && (*a)->right->right->level == (*a)->level){
		TST_DBG("at %x->%x->%x:%d\n", (*a)->key, (*a)->right->key, (*a)->right->right->key, (*a)->level);
		//left rotate
		struct Node *b = (*a)->right;
		(*a)->right = b->left;
		b->left = *a;
		*a = b;
		
		(*a)->level++; //increase level
	}
}

#else

struct Node *tree_nil(void)
{
	return NULL;
}
#endif

#if USE_GROUP_ALLOC
#define NR (1*32)
struct Group{
	struct Node nodes[NR];
	unsigned int allocMsk[NR/32];
	struct Group *next;
};

static struct Group *s_freeList = NULL;

static void* group_alloc(void)
{
	struct Group *g;
	struct Node *ptr = NULL;
	unsigned i;
	
	for(g = s_freeList; g; g = g->next){
		for(i = 0; i < NR; ++i){
			if(!(g->allocMsk[i/32]&(1<<(i%32)))){
				g->allocMsk[i/32] |= (1<<(i%32));
				ptr = &g->nodes[i];
				return ptr;
			}
		}

	}

	g = malloc(sizeof *g);
	if(g){
		memset(g, 0, sizeof *g);
		g->allocMsk[0] |= 1;
		ptr = &g->nodes[0];
	
		g->next = s_freeList;
		s_freeList = g;
	}

	return ptr;
}

static void group_free(struct Node *ptr)
{
	struct Group *g, *prev = NULL;
	unsigned  j, i;
	
	for(g = s_freeList; g; g = g->next){
		if(g->nodes+0 <= ptr && ptr < g->nodes+NR){
			i = (ptr - g->nodes)/sizeof(g->nodes[0]);
			g->allocMsk[i/32] &= (~(1<<(i%32)));
			break;
		}
		prev = g;
	}

	if(!g){
		return;
	}
	

	#if 0
	(void)prev;
	(void)j;
	#else
	
	for(j = 0; j < NR/32; ++j){
		if(g->allocMsk[j]){
			break;
		}
	}

	if(j >= NR/32){
		if(g == s_freeList){
			s_freeList = g->next;
		}else{
			prev->next = g->next;
		}
		g->next = NULL;
				
		free(g);
	}
	#endif
}

static void group_drop(void)
{
	struct Group *g, *g2;
	for(g = s_freeList; g; g = g2){
		g2 = g->next;
		free(g);
		if(g == s_freeList){
			s_freeList = NULL;
		}
	}
}

#endif
 
static struct Node* newNode(node_key_t key, void *val)
{
    struct Node *temp;
	#if USE_GROUP_ALLOC
	temp = group_alloc();
	#else
	temp = malloc(sizeof *temp);
	#endif
	if(!temp){
		return tree_nil();
	}
	
	temp->key = key;
	temp->val = val;
    temp->isEnd = 0;
	#if CONFIG_REBALANCE_TREE
	temp->level = 1;
	#endif
    temp->left = temp->eq = temp->right = tree_nil();
    return temp;
}

static void freeNode(struct Node *ptr)
{
	if(ptr != tree_nil()){
		#if USE_GROUP_ALLOC
		group_free(ptr);
		#else
		free(ptr);
		#endif
	}
}
 
void tree_insert(struct Node** root, node_key_t *keys, int num, void *val)
{
	if(!keys || num < 1){
		return;
	}
	
    if (*root == tree_nil()){
        *root = newNode(*keys, NULL);
	}

	if(*root == tree_nil()){
		return;
	}
 
    if ((*keys) < (*root)->key){
        tree_insert(&(*root)->left, keys, num, val);
    }else if ((*keys) > (*root)->key){
        tree_insert(&(*root)->right, keys, num, val);
    }else{
        if (num > 1){
            tree_insert(&(*root)->eq, keys+1, num-1, val);
        }else{
            (*root)->isEnd = 1;
			(*root)->val = val;
		}
    }

	#if CONFIG_REBALANCE_TREE
	if((*keys) != (*root)->key){
		skew(root);
	    split(root);
	}
	#endif
}

static struct Node* find(struct Node *root, node_key_t *keys, int num)
{
	if(root == tree_nil() || !keys || num < 1){
		return tree_nil();
	}
 
    if (*keys < root->key){
        return find(root->left, keys, num);
    }else if (*keys > root->key){
        return find(root->right, keys, num);
	}else{
        if (num == 1)
            return root;
        return find(root->eq, keys+1, num-1);
    }
}

static void traverse(struct Node* root, struct TraverseCtx *ctx, int depth)
{
    if (root != tree_nil()) {
        traverse(root->left, ctx, depth);

 		if(ctx){
			if(ctx->path && depth < ctx->pathLen){
	        	ctx->path[depth] = root->key;
			}
			
	        if (root->isEnd) {
				if(ctx->cb){
					ctx->cb(ctx, depth+1, root->val);
				}
	        }
 		}
 
		traverse(root->eq, ctx, depth+1);
 
		traverse(root->right, ctx, depth);
    }
}

static void deleteR(struct Node *root, FreeFn fn)
{
	if(root != tree_nil()){
		deleteR(root->left, fn);
		deleteR(root->eq, fn);
		deleteR(root->right, fn);

		if(fn){
			fn(root->val);
		}
		freeNode(root);
	}
}

static void node_delete(struct Node **root, node_key_t key, FreeFn fn)
{
	if ((*root) == tree_nil()){
		return;
	}

	int diff = key - (*root)->key;
    if (diff == 0) {
        if ((*root)->left != tree_nil() && (*root)->right != tree_nil()) {
			//remove the node at level 1
			struct Node *prev = (*root);
            struct Node *heir = (*root)->left;
			
            while (heir->right != tree_nil()){
				prev = heir;
                heir = heir->right;
            }

            (*root)->key = heir->key;

			if(fn){
				fn((*root)->val);
			}
			(*root)->val = heir->val;
            
			if(prev == (*root)){
				prev->right = heir->right;
			}else{
				prev->left = heir->right;
			}
			freeNode(heir);
			
        } else {//single child case
			struct Node *node = *root;
			(*root) = (*root)->left != tree_nil() ? (*root)->left : (*root)->right;
			
			if(fn){
				fn(node->val);
			}
			freeNode(node);
        }
    } else if(diff < 0){
		node_delete(&(*root)->left, key, fn);
	}else{
		node_delete(&(*root)->right, key, fn);
	} 

	#if CONFIG_REBALANCE_TREE
    if ((*root)->left->level < (*root)->level - 1
		|| (*root)->right->level < (*root)->level - 1) {
		//walk right and rebalance
        if ((*root)->right->level > --(*root)->level) {
			//root and root->right are in the same pseudo-node.
            (*root)->right->level = (*root)->level;
        }

		skew(root);
		skew(&(*root)->right);
		skew(&(*root)->right->right);
        split(root);
		split(&(*root)->right);
    }
	#endif
}

void* tree_search(struct Node *root, node_key_t *keys, int num)
{
	if(root == tree_nil() || !keys || num < 1){
		return NULL;
	}

	struct Node *node = find(root, keys, num);
	return node != tree_nil() ? node->val : NULL;
}

int tree_traverse(struct Node* root, node_key_t *keys, int num, 
		struct TraverseCtx *ctx)
{
	int ret = 0;
	if(root == tree_nil() || !keys || num < 1){
		traverse(root, ctx, 0);
	}else{
		struct Node *node = find(root, keys, num);
		if(node != tree_nil()){
			traverse(node->eq, ctx, 0);
		}else{
			ret = -1;
		}
	}
	
	return ret;
}


int tree_delete(struct Node **root, node_key_t *keys, int num, int flag, FreeFn fn)
{
	struct Node *fa = *root, *ch = tree_nil();
	struct Node **link = NULL;
	int i, ret = 0;
	struct Node **last_link = NULL;
	if(fa == tree_nil()){
		return -1;
	}

	if(!keys){
		deleteR(fa, fn);
		*root = tree_nil();
#if USE_GROUP_ALLOC
		group_drop();
#endif
		return 0;
	}
	
	ch = fa;
	i = 0;
	while(i < num && ch != tree_nil()){
		if(keys[i] == ch->key){
			if(i+1 == num){
				i++;
				break;
			}
			fa = ch;
			ch = fa->eq;
			i++;
			
			if(i+1 == num){
				last_link = &fa;
			}
		}else if(keys[i] < ch->key){
			fa = ch;
			ch = fa->left;
		}else{
			fa = ch;
			ch = fa->right;
		}
	}

	if(i < num || ch == tree_nil()){//not found
		return -2;
	}

	//the link between father and child.
	if(fa->eq == ch){
		link = &fa->eq;
	}else if(fa->left == ch){
		link = &fa->left;
	}else if(fa->right == ch){
		link = &fa->right;
	}else if(fa == ch){
		link = root;
	}

	if(0 == flag){
		//need exact match
		if(ch->isEnd){
			if(fn){
				fn(ch->val);
			}

			if(ch->eq != tree_nil()){
				//has longer keys
				ch->isEnd = 0;
				ret = 1;
			}else{
				node_delete(last_link, keys[num-1], fn);
			}

		}else{
			ret = -3; //partial match
		}
		
	}else if(1 == flag){
		//need match prefix
		ch->isEnd = 0;
		if(ch->left != tree_nil() || ch->right != tree_nil()){
			//has brothers
			deleteR(ch->eq, fn);
			ch->eq = tree_nil();
		}else{
			*link = tree_nil();
			deleteR(ch, fn);
		}
	}else{
		ret = -4;
	}

	if(*root == tree_nil()){
#if USE_GROUP_ALLOC
		group_drop();
#endif
	}

	return ret;
}


int tree_compare_and_set(struct Node *root, node_key_t *keys, int num, struct CompareCtx *ctx)
{
	if(!root || !keys || num < 1 || !ctx || !ctx->cmpFn){
		return 0;
	}

	struct Node *node = find(root, keys, num);
	if(node != tree_nil()){
		if(ctx->cmpFn(node->val, ctx->cmpTo)){
			if(ctx->freeFn){
				ctx->freeFn(node->val);
			}
			node->val = ctx->cmpTo;
			return 1;
		}
	}
	return 0;
}


#ifdef test_ternary_search_tree
#include <stdio.h>

static int string2key(char *str, node_key_t *keys)
{
	char *orig = str;
	
	while(*str){
		*keys++ = *str++;
	}
	
	return str - orig;
}

static void print(struct TraverseCtx *ctx, int curPathLen, void *val)
{
	int i;
	for(i = 0; i < curPathLen; ++i){
		printf("%c", (char)ctx->path[i]);
	}
	printf(" : %lu\n", (unsigned long)val);
}

static void freeVal(void *ptr)
{
	(void)ptr;
}

int test_ternary_search_tree()
{
    struct Node *root = tree_nil();
	char *str;

	node_key_t keys[16];
	int keylen;
	struct TraverseCtx ctx = {0};
	ctx.cb = print;
	ctx.path = keys;
	ctx.pathLen = sizeof(keys)/sizeof(keys[0]);
	ctx.userData = (void*)1;

	TST_DBG("");
	keylen = string2key("cat", keys);
	tree_insert(&root, keys, keylen, (void*)1);

	TST_DBG("");
	keylen = string2key("cats", keys);
	tree_insert(&root, keys, keylen, (void*)2);

	TST_DBG("");
	keylen = string2key("up", keys);
	tree_insert(&root, keys, keylen, (void*)3);

	TST_DBG("");
	keylen = string2key("bug", keys);
	tree_insert(&root, keys, keylen, (void*)4);

	keylen = string2key("c", keys); //delete a non-exist key
	tree_delete(&root, keys, keylen, 0, freeVal);

	printf("====dump all\n");
    tree_traverse(root, NULL, 0, &ctx);

	printf("====tree_search key:val==\n");
	str = "cats";
	keylen = string2key(str, keys);
	printf("%s:%lu\n", str, (unsigned long)tree_search(root, keys, keylen));

	str = "bu";
	keylen = string2key(str, keys);
	printf("%s:%lu\n", str, (unsigned long)tree_search(root, keys, keylen));

	str = "cat";
	keylen = string2key(str, keys);
	printf("%s:%lu\n", str, (unsigned long)tree_search(root, keys, keylen));

	str = "cat";
	keylen = string2key(str, keys);
	tree_delete(&root, keys, keylen, 0, freeVal);
	printf("====after remove %s\n", str);
	tree_traverse(root, NULL, 0, &ctx);

	str = "cats";
	keylen = string2key(str, keys);
	tree_delete(&root, keys, keylen, 0, freeVal);
	printf("====after remove %s\n", str);
	tree_traverse(root, NULL, 0, &ctx);

	tree_delete(&root, NULL, 0, 0, freeVal);
	printf("====after remove all %p\n", root);
	tree_traverse(root, NULL, 0, &ctx);
	
    return 0;
}
#endif

