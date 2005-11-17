/* rb.h */
/* This code was written by Dongshin Kim, at Iowa State University
   under supervision by Dianne Cook */

#ifndef RBNODE
#define RBNODE

#include <gtk/gtk.h>

typedef gshort bool;

#if !defined(TRUE)
#define true  ((bool) 1)
#define false ((bool) 0)
#endif

typedef enum color {Black, Red} NodeColor;

/* A Node structure. In addition to the left/right and parent
 * pointers you would also include the augmented data.
*/

typedef struct RBNode
{
    struct RBNode *Left;         /* left child */
    struct RBNode *Right;        /* right child */
    struct RBNode *Parent;       /* parent */


    NodeColor Color;             /* node color (black, red) */
    gint index;	
    char key[255];
}Node;

#endif
#ifndef RB_TREE
#define RB_TREE
typedef struct RB_Tree
{
	Node * NIL;
	Node * Root;
}Tree;
#endif

Node* GetNIL(Tree* T);
Node* Search(Tree* T, Node* X, char* key);
void DeleteFixup(Tree* T, Node* x);
void copy_key(Tree* T, Node* X, Node* Y);
Node* Minimum(Tree* T, Node* X);
Node* Successor(Tree* T, Node* X);
Node* DeleteNode(Tree* T, Node* Z);
bool IsEmpty(Tree* T);
gint max(gint x, gint y);
Node* GetRoot(Tree* T);
void Print(Node* X);
void InorderTravel(Tree* T, Node* X);	
	
void InitRB_Tree(Tree* T);
	
void CloseRB_Tree(Tree* T);
	
Node * InsertNode(Tree* T, Node* Z);
	
void InsertFixup(Tree* T, Node * Z);
	
void RotateLeft(Tree* ,Node *);
	
void RotateRight(Tree* ,Node *);
	
void Kill_Tree(Tree* T, Node *);
