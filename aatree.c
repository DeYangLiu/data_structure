//http://www.eternallyconfuzzled.com/tuts/datastructures/jsw_tut_andersson.aspx
#include <stdlib.h>
#include <stdio.h>

#define LOG(fmt, ...) fprintf(stdout, "[%s %d]"fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); 

/*view binary search tree as multi-way search tree:
		4
   /		 \
  2 		  10
 / \	   /	  \
1	3	  6 		12
		 / \	   /  \
		5	8	 11    13
		   / \
		  7   9


     (4-------------10)
    /        |         \
   2      (6---8)       12
  / \     /  | \      /  \
 1   3   5   7   9   11    13

by group (4,10), (6,8) as pseudo-nodes.

after a deletion, the worst case: 
 2,1
    \
     5,1
    /   \
 3,1     6,1
    \       \
     4,1     7,1

needs skew 2, skew 5, skew 5, split 2, split 4, so recusive functions
can be expanded as: skew(t), skew(t.right), skew(t.right.right);
split(t), split(t.right); where 
skew(root){
 if horizontal left
   right rotatation
 //skew(root->right)
}

split(root){
 if consecutive right
  left rotatation
  //split(root->right);
}

*/
struct Node
{
    int data;
    int level;
    struct Node *left, *right;
};

static struct Node s_nil = {.left = &s_nil, .right = &s_nil};
static struct Node *nil = &s_nil;


struct Node *newNode(int data, int level)
{
    struct Node *rn = malloc(sizeof *rn);
    if (!rn) {
        return NULL;
    }

    rn->data = data;
    rn->level = level;
    rn->left = rn->right = nil;
    return rn;
}


void skew(struct Node **a)
{
    if ((*a)->level != 0) {
		//remove left horizontal links
        if ((*a)->left->level == (*a)->level) {
			LOG("at %x->%x:%d\n", (*a)->data, (*a)->left->data, (*a)->level);
			//right rotate
			struct Node *b = (*a)->left;
			(*a)->left = b->right;
			b->right = *a;
			*a = b;
        }
    }
}

void split(struct Node **a)
{
	//remove consecutive right horizontal links.
	if((*a)->level != 0 && (*a)->right->right->level == (*a)->level){
		LOG("at %x->%x->%x:%d\n", (*a)->data, (*a)->right->data, (*a)->right->right->data, (*a)->level);
		//left rotate
		struct Node *b = (*a)->right;
		(*a)->right = b->left;
		b->left = *a;
		*a = b;
		
		(*a)->level++; //increase level
	}
}

void insert(struct Node **root, int data)
{
    if ((*root) == nil) {
        (*root) = newNode(data, 1); //add new node at level 1
    } else {

		if(data < (*root)->data){
			insert(&(*root)->left, data);
		}else if(data > (*root)->data){
			insert(&(*root)->right, data);
		}

        skew(root);
        split(root);
    }
}


void delete(struct Node **root, int data)
{
    if ((*root) != nil) {
		int diff = data - (*root)->data;
		
        if (diff == 0) {
            if ((*root)->left != nil && (*root)->right != nil) {
				
				//remove the node at level 1
				struct Node *prev = (*root);
                struct Node *heir = (*root)->left;
				
                while (heir->right != nil){
					prev = heir;
                    heir = heir->right;
                }

                (*root)->data = heir->data;
                
				if(prev == (*root)){
					prev->right = heir->right;
				}else{
					prev->left = heir->right;
				}
				free(heir);
				
            } else {//single child case
				struct Node *node = *root;
				(*root) = (*root)->left != nil ? (*root)->left : (*root)->right;
				free(node);
            }
        } else if(diff < 0){
			delete(&(*root)->left, data);
		}else{
			delete(&(*root)->right, data);
		}  
    }

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
}

void delete_iter(struct Node **root, int data)
{
	if(*root == nil){
		return;
	}
	
    struct Node *it = *root;
    struct Node *up[32];
    int top = 0;

    for (;;){
        up[top++] = it;

        if (it == nil) {
            return;
        } else if (data == it->data){
            break;
        }

		if(data < it->data){
			 it = it->left;
		}else{
			it = it->right;
		}
    }

    if (it->left == nil || it->right == nil){

        if (--top != 0){
			struct Node **link = (up[top-1]->left == it) ? &up[top-1]->left : &up[top-1]->right;
			if(it->left != nil){
				*link = it->left;
			}else{
				*link = it->right;
			}

        }else {
            *root = it->right;
        }
		free(it);
		
    } else{
        struct Node *heir = it->right;
        struct Node *prev = it;

        while (heir->left != nil) {
            up[top++] = prev = heir;
            heir = heir->left;
        }

        it->data = heir->data;
		if(prev == it){
			prev->right = heir->right;
		}else{
			prev->left = heir->right;
		}

		free(heir);
    }

    while (--top >= 0){
        if (up[top]->left->level < up[top]->level - 1 
			|| up[top]->right->level < up[top]->level - 1){
            if (up[top]->right->level > --up[top]->level) {
                up[top]->right->level = up[top]->level;
            }

            skew(&up[top]);
			skew(&up[top]->right);
			skew(&up[top]->right->right);
            split(&up[top]);
			split(&up[top]->right);
        }

        if (top == 0){
            *root = up[top];
        }
    }

}

void destroy(struct Node **root)
{
	struct Node *b, *a = *root;
	for(; a != nil; a = b){
		if(a->left == nil){
			b = a->right;
			free(a);
		}else{//right rotate
			b = a->left;
			a->left = b->right;
			b->right = a;
		}
	}
	*root = nil;
}

void traverse(struct Node *root, int depth)
{
	int i;
	if(root != nil){
		
		traverse(root->left, depth+1);
		
		for(i = 0; i < depth; ++i)
			printf(" ");
		printf("%d:%d\n", root->data, root->level);

		traverse(root->right, depth+1);
	}
}

#if defined(test_aatree)
int test_aatree()
{
	struct Node *root = nil;
	int i;

	for(i = 1; i <= 6; ++i){
		insert(&root, i);
	}

	traverse(root, 0);

	for(i = 1; i <= 6; ++i){
		LOG("begin rm %d:\n", i);
		//delete(&root, i);
		delete_iter(&root, i);
		
		LOG("after rm %d:\n", i);
		traverse(root, 0);
	}

	//destroy(&root);
	//LOG("====end:\n");
	//traverse(root, 0);

	
	return 0;
}
#endif
