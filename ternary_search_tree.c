#include <stdlib.h>
#include "ternary_search_tree.h"

struct Node
{
	void *val;
    node_key_t key;
	
    char isEnd;
    struct Node *left, *eq, *right;
};
 
static struct Node* newNode(node_key_t key, void *val)
{
    struct Node *temp =  malloc(sizeof *temp);
	
	temp->key = key;
	temp->val = val;
    temp->isEnd = 0;
    temp->left = temp->eq = temp->right = NULL;
    return temp;
}

static void freeNode(struct Node *ptr)
{
	if(ptr){
		free(ptr);
	}
}
 
void tree_insert(struct Node** root, node_key_t *keys, int num, void *val)
{
	if(!keys || num < 1){
		return;
	}
	
    if (!*root){
        *root = newNode(*keys, NULL);
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
}

static struct Node* find(struct Node *root, node_key_t *keys, int num)
{
	if(!root || !keys || num < 1){
		return NULL;
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
    if (root) {
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
	if(root){
		deleteR(root->left, fn);
		deleteR(root->eq, fn);
		deleteR(root->right, fn);

		if(fn){
			fn(root->val);
		}
		freeNode(root);
	}
}

void* tree_search(struct Node *root, node_key_t *keys, int num)
{
	if(!root || !keys || num < 1){
		return NULL;
	}

	struct Node *node = find(root, keys, num);
	return node ? node->val : NULL;
}

void tree_traverse(struct Node* root, node_key_t *keys, int num, 
		struct TraverseCtx *ctx)
{
	if(!root || !keys || num < 1){
		traverse(root, ctx, 0);
	}else{
		struct Node *node = find(root, keys, num);
		if(node){
			traverse(node->eq, ctx, 0);
		}
	}
}


int tree_delete(struct Node **root, node_key_t *keys, int num, int flag, FreeFn fn)
{
	struct Node *fa = *root, *ch = NULL;
	struct Node **link = NULL;
	int i, ret = 0;
	if(!fa){
		return -1;
	}

	if(!keys){
		deleteR(fa, fn);
		*root = NULL;
		return 0;
	}
	
	ch = fa;
	i = 0;
	while(i < num && ch){
		if(keys[i] == ch->key){
			if(i+1 == num){
				i++;
				break;
			}
			fa = ch;
			ch = fa->eq;
			i++;
		}else if(keys[i] < ch->key){
			fa = ch;
			ch = fa->left;
		}else{
			fa = ch;
			ch = fa->right;
		}
	}

	if(i < num || !ch){//not found
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
			
			if((ch->eq || ch->left || ch->right)){
				//has longer keys, or brothers.
				ch->isEnd = 0;
				ret = 1;
			}else{
				*link = NULL;
				freeNode(ch);
				ret = 0;
			}
		}else{
			ret = -3; //partial match
		}
		
	}else if(1 == flag){
		//need match prefix
		ch->isEnd = 0;
		if(ch->left || ch->right){
			//has brothers
			deleteR(ch->eq, fn);
			ch->eq = NULL;
		}else{
			*link = NULL;
			deleteR(ch, fn);
		}
	}else{
		ret = -4;
	}

	return ret;
}


int tree_compare_and_set(struct Node *root, node_key_t *keys, int num, struct CompareCtx *ctx)
{
	if(!root || !keys || num < 1 || !ctx || !ctx->cmpFn){
		return 0;
	}

	struct Node *node = find(root, keys, num);
	if(node){
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
    struct Node *root = NULL;
	char *str;
	int ret;

	node_key_t keys[16];
	int keylen;
	struct TraverseCtx ctx = {0};
	ctx.cb = print;
	ctx.path = keys;
	ctx.pathLen = sizeof(keys)/sizeof(keys[0]);
	ctx.userData = (void*)1;

	keylen = string2key("cat", keys);
	tree_insert(&root, keys, keylen, (void*)1);

	keylen = string2key("cats", keys);
	tree_insert(&root, keys, keylen, (void*)2);

	keylen = string2key("up", keys);
	tree_insert(&root, keys, keylen, (void*)3);

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

