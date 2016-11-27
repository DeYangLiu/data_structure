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
 
void insert(struct Node** root, node_key_t *keys, int num, void *val)
{
	if(!keys || num < 1){
		return;
	}
	
    if (!*root){
        *root = newNode(*keys, NULL);
	}
 
    if ((*keys) < (*root)->key){
        insert(&(*root)->left, keys, num, val);
    }else if ((*keys) > (*root)->key){
        insert(&(*root)->right, keys, num, val);
    }else{
        if (num > 1){
            insert(&(*root)->eq, keys+1, num-1, val);
        }else{
            (*root)->isEnd = 1;
			(*root)->val = val;
		}
    }
}

void* search(struct Node *root, node_key_t *keys, int num)
{
	if(!root || !keys || num < 1){
		return NULL;
	}
 
    if (*keys < root->key){
        return search(root->left, keys, num);
    }else if (*keys > root->key){
        return search(root->right, keys, num);
	}else{
        if (num == 1)
            return root->val;
        return search(root->eq, keys+1, num-1);
    }
}
 
void traverse(struct Node* root, TraverseFn fn, node_key_t* path, int depth)
{
    if (root) {
        traverse(root->left, fn, path, depth);
 
        path[depth] = root->key;
        if (root->isEnd) {
			if(fn){
				fn(path, depth+1, root->val);
			}
        }
 
		traverse(root->eq, fn, path, depth+1);
 
		traverse(root->right, fn, path, depth);
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

int delete(struct Node **root, node_key_t *keys, int num, FreeFn fn)
{
	struct Node *fa = *root, *ch = NULL;
	int i;
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

	if(i < num || !ch || !ch->isEnd){//not found
		return -2;
	}

	ch->isEnd = 0;
	if(ch->eq){//has longer keys.
		return 1;
	}

	if(fa->eq == ch){
		fa->eq = NULL;
	}else if(fa->left == ch){
		fa->left = NULL;
	}else if(fa->right == ch){
		fa->right = NULL;
	}else if(fa == ch){
		*root = NULL;
	}
			
	deleteR(ch, fn);
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

static void print(node_key_t *path, int len, void *val)
{
	int i;
	for(i = 0; i < len; ++i){
		printf("%c", (char)path[i]);
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

	node_key_t keys[16];
	int keylen;

	keylen = string2key("cat", keys);
	insert(&root, keys, keylen, (void*)1);

	keylen = string2key("cats", keys);
	insert(&root, keys, keylen, (void*)2);

	keylen = string2key("up", keys);
	insert(&root, keys, keylen, (void*)3);

	keylen = string2key("bug", keys);
	insert(&root, keys, keylen, (void*)4);
	
	keylen = string2key("c", keys);
	delete(&root, keys, keylen, freeVal);
	printf("====dump all\n");
    traverse(root, print, keys, 0);

	printf("====search key:val==\n");
	str = "cats";
	keylen = string2key(str, keys);
	printf("%s:%lu\n", str, (unsigned long)search(root, keys, keylen));

	str = "bu";
	keylen = string2key(str, keys);
	printf("%s:%lu\n", str, (unsigned long)search(root, keys, keylen));

	str = "cat";
	keylen = string2key(str, keys);
	printf("%s:%lu\n", str, (unsigned long)search(root, keys, keylen));

	str = "cat";
	keylen = string2key(str, keys);
	delete(&root, keys, keylen, freeVal);
	printf("====after remove %s\n", str);
	traverse(root, print, keys, 0);

	str = "cats";
	keylen = string2key(str, keys);
	delete(&root, keys, keylen, freeVal);
	printf("====after remove %s\n", str);
	traverse(root, print, keys, 0);

	delete(&root, NULL, 0, freeVal);
	printf("====after remove all %p\n", root);
	traverse(root, print, keys, 0);
	
    return 0;
}
#endif
