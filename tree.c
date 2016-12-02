#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "tree.h"

void tree_traverse(struct Node *root, TraverseFn fn, void *ctx)
{
	struct Node *ch, *sb = NULL;

	if(!root) {
		return;
	}

	if(!root->child) { //leaf node
		if(fn) {
			fn(ctx, root->val);
		}
	} else {
		ch = root->child;
		for(sb = ch; sb; sb = sb->sibling) {
			tree_traverse(sb, fn, ctx);
		}
	}
}

struct Node* tree_find(struct Node *root, U16 keys[], int num)
{
	int i;
	struct Node *ch, *sb = NULL;

	if(!root) {
		return NULL;
	}

	ch = root->child;
	for(i = 0; i < num; ++i) {
		for(sb = ch; sb; sb = sb->sibling) {
			if(sb->key == keys[i]) {
				break;
			}
		}

		if(!sb) {
			return NULL;
		} else {
			ch = sb->child;
		}
	}
	return sb;
}


int tree_insert(struct Node *root, U16 keys[], int num, void *val)
{
	int i;
	struct Node *fa, *ch, *sb;

	if(!root) {
		return -2;
	}
	fa = root;
	ch = fa->child;

	for(i = 0; i < num; ++i) {
		for(sb = ch; sb; sb = sb->sibling) {
			if(sb->key == keys[i]) {
				break;
			}
		}

		if(sb) {
			fa = sb; //found, check next key
		} else {
			//add the key to the head
			fa = fa->child = tree_newNode(keys[i], NULL, ch, NULL);
			if(!fa) {
				return -1;
			}
		}

		ch = fa->child;
	}

	fa->val = val;
	return 0;
}

static int deleteR(struct Node *root, NodeFn freeVal)
{
	struct Node *sb, *s2, *ch, *fa = root;

	if(fa) {
		ch = fa->child;
		for(sb = ch; sb; sb = s2) {
			s2 = sb->sibling;

			//fa->child = s2;
			//sb->sibling = NULL;
			deleteR(sb, freeVal);
		}

		if(freeVal && fa->val) {
			freeVal(fa->val);
			fa->val = NULL;
		}

		tree_freeNode(fa);
		return 0;
	} else {
		return -1;
	}
}

int tree_delete(struct Node *root, U16 keys[], int num, NodeFn freeVal)
{
	struct Node *fa, *sb, *s2, *ch, *prev;
	int i;
	if(!keys && !num) { //tree_delete all except root
		fa = root;
		if(fa) {
			for(sb = fa->child; sb; sb = s2) {
				s2 = sb->sibling;
				
				//fa->child = s2;
				//sb->sibling = NULL;
				deleteR(sb, freeVal);
			}
			if(freeVal && fa->val) {
				freeVal(fa->val);
				fa->val = NULL;
			}
			
			fa->child = NULL;
			return 0;
		} else {
			return -1;
		}
	} else {
		if(!root || num < 1){
			return -1;
		}

		fa = root;
		sb = NULL;
		ch = fa->child;
		
		for(i = 0; i < num; ++i) {
			for(sb = ch; sb; sb = sb->sibling) {
				if(sb->key == keys[i]) {
					break;
				}
				
			}

			if(!sb){
				return -2;
			}

			if(i == num -1){
				//keep fa and sb
				break;
			}else{
				fa = sb; //found, check next key
				ch = fa->child;
			}
		}
		//here assert(fa && sb)

		//remove from fa's child list
		if(fa->child == sb){
			fa->child = sb->sibling;
			sb->sibling = NULL;
		}else{
			for(prev = ch; prev; prev = prev->sibling){
				if(prev->sibling == sb){
					break;
				}
			}
			
			//assert(prev->next == sb);
			prev->sibling = sb->sibling;
			sb->sibling = NULL;
		}
		deleteR(sb, freeVal);
		
		return 0;
	}
}

////mem && gc
struct Node *tree_newNode(int key, void *val, struct Node *sibling, struct Node *child)
{
	struct Node *node = malloc(sizeof *node);
	if(node) {
		node->key = key;
		node->val = val;
		node->sibling = sibling;
		node->child = child;
	}

	return node;
}

void tree_freeNode(struct Node *node)
{
	if(node) {
		free(node);
	}
}

