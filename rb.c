/* rb.c */
/* This code was written by Dongshin Kim, at Iowa State University
   under supervision by Dianne Cook */
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "rb.h"


void InitRB_Tree(Tree* T)
{
  T->NIL = (Node*)malloc(sizeof(Node));
  T->NIL->Left = T->NIL->Right = T->NIL->Parent = NULL;
  T->NIL->Color = Black;
  T->Root = T->NIL;
} 


void CloseRB_Tree(Tree* T)
{
  Kill_Tree(T,T->Root);
  free(T->NIL);
} 


Node * InsertNode(Tree* T, Node* Z)
{
  Node* Y;
  Node* X;
  Z->Left = Z->Right = Z->Parent = T->NIL;
  Y = T->NIL;
  X = T->Root;
  
  while(X != T->NIL)
  {
    Y = X;
    if(strcmp(Z->key,X->key)<0)
      X = X->Left;
    else
      X = X->Right;
  }
  Z->Parent = Y;
  if (Y == T->NIL)
  {
    T->Root = Z;
  }
  else
  {
    if(strcmp(Z->key,Y->key)<0)
      Y->Left = Z;
    else
      Y->Right = Z;
  }
  return Z;
}

Node * AppendNode(Tree* T, Node* Z)
{
  Node* Y;
  Node* X;
  Z->Left = Z->Right = Z->Parent = T->NIL;
  Y = T->NIL;
  X = T->Root;
  
  while(X != T->NIL)
  {
    Y = X;
    if(strcmp(Z->key,X->key)<0)
      X = X->Left;
    else
      X = X->Right;
  }
  Z->Parent = Y;
  if (Y == T->NIL)
  {
    T->Root = Z;
  }
  else
  {
    if(strcmp(Z->key,Y->key)<0)
      Y->Left = Z;
    else
      Y->Right = Z;
  }
  return Z;
}

void InsertFixup(Tree* T, Node * X)
{
  Node* Y;
  InsertNode(T, X);  
  X->Color = Red;

  while (X != T->Root && X->Parent->Color == Red)
  {
      if (X->Parent == X->Parent->Parent->Left)
      {
        Y = X->Parent->Parent->Right;
        if (Y->Color == Red)
        {   
          X->Parent->Color = Black;
          Y->Color = Black;
          X->Parent->Parent->Color = Red;
          X = X->Parent->Parent;
        }
        else
        {
          if (X == X->Parent->Right)
          {
            X = X->Parent;
            RotateLeft(T,X);
          }
          X->Parent->Color = Black;
          X->Parent->Parent->Color = Red;
          RotateRight(T,X->Parent->Parent);
        }
      }
      else
      { 
        Node *Y = X->Parent->Parent->Left;
        if (Y->Color == Red)
        {  
          X->Parent->Color = Black;
          Y->Color = Black;
          X->Parent->Parent->Color = Red;
          X = X->Parent->Parent;
        }
        else
        { 
          if (X == X->Parent->Left)
          {
            X = X->Parent;
            RotateRight(T,X);
          }
          X->Parent->Color = Black;
          X->Parent->Parent->Color = Red;
    
          
          RotateLeft(T,X->Parent->Parent);
        }
      }
  } 
  T->Root->Color = Black;
}


void RotateLeft(Tree* T, Node *X)
{
  Node *Y = X->Right; 
  X->Right = Y->Left;
  if (Y->Left != T->NIL) Y->Left->Parent = X; 
  Y->Parent = X->Parent;
  if (X->Parent == T->NIL)
    T->Root = Y;
  else if( X == X->Parent->Left)
    X->Parent->Left = Y;
  else
    X->Parent->Right = Y;
  Y->Left = X;
  X->Parent = Y;
}


void RotateRight(Tree* T, Node *X)
{
  Node *Y = X->Left; 
  X->Left = Y->Right;
  if (Y->Right != T->NIL) Y->Right->Parent = X; 
  Y->Parent = X->Parent;
  if (X->Parent == T->NIL)
    T->Root = Y;
  else if(X == X->Parent->Right)
  {
    X->Parent->Right = Y;
  }
  else
  {
    X->Parent->Left = Y;
  }

  Y->Right = X;
  X->Parent = Y;
}

void Kill_Tree(Tree* T, Node *start)
{
  if (start != T->NIL)
  {
    Kill_Tree(T, start->Left);
    Kill_Tree(T, start->Right);
    memset(start->key,'\0',255);
    start->Left = start->Right = start->Parent = NULL;
    free(start);

  }
}



void InorderTravel(Tree* T, Node* X)
{
  if(X != T->NIL)
  {
    InorderTravel(T, X->Left);
    Print(X);
    InorderTravel(T, X->Right);
  }
}

void Print(Node* X)
{
  printf("Key: %s\t", X->key);
}
Node* GetRoot(Tree* T)
{
  return T->Root;
}

Node* GetNIL(Tree* T)
{
  return T->NIL;
}

gint max(gint x, gint y)
{
  if(x < y)
    return y;
  else
    return x;
}

bool IsEmpty(Tree* T)
{
  if(T->Root == T->NIL)
    return (gshort) 1; /*true;*/
  else
    return (gshort) 0; /*false;*/
}

Node* DeleteNode(Tree* T, Node* Z)
{
  Node* Y;
  Node* X;
  if(Z->Left == T->NIL || Z->Right == T->NIL)
    Y = Z;
  else
    Y = Successor(T,Z);
  if (Y->Left != T->NIL)
    X = Y->Left;
  else
    X = Y->Right;
  X->Parent = Y->Parent;
  if(Y->Parent == T->NIL)
    T->Root = X;
  else
  {
    if (Y == Y->Parent->Left)
      Y->Parent->Left = X;
    else
      Y->Parent->Right = X;
  }
  if(Y != Z)
    copy_key(T,Y,Z);
  if (Y->Color == Black)
    DeleteFixup(T,X);
  return Y;
}

void copy_key(Tree* T, Node* X/*source*/, Node* Y/*target*/)
{
  Y->Color = X->Color;
  strcpy(Y->key,X->key);
}

void DeleteFixup(Tree* T, Node* x)
{
  Node * w;
  Node * rootLeft = T->Root;
  
  while( (x->Color == Black) && (rootLeft != x))
  {
    if (x == x->Parent->Left)
    {
      w=x->Parent->Right;
      if(w == NULL)
        return;
      if (w->Color == Red)
      {
        w->Color = Black;
        x->Parent->Color = Red;
        RotateLeft(T,x->Parent);
        w=x->Parent->Right;
      }
      if ( (w->Right->Color == Black) && (w->Left->Color == Black) )
      {
        w->Color = Red;
        x=x->Parent;
      }
      else
      {
        if (w->Right->Color == Black)
        {
          w->Left->Color = Black;
          w->Color = Red;
          RotateRight(T,w);
          w=x->Parent->Right;
        }
        w->Color=x->Parent->Color;
        x->Parent->Color = Black;
        w->Right->Color = Black;
        RotateLeft(T,x->Parent);
        x=rootLeft;
      }
    }
    else
    {
      w=x->Parent->Left;
      if (w->Color == Red)
      {
        w->Color = Black;
        x->Parent->Color = Red;
        RotateRight(T,x->Parent);
        w=x->Parent->Left;
      }
      if ( (w->Right->Color == Black) && (w->Left->Color == Black) )
      {
        w->Color = Red;
        x=x->Parent;
      }
      else
      {
        if (w->Left->Color == Black)
        {
          w->Right->Color= Black;
          w->Color = Red;
          RotateLeft(T,w);
          w=x->Parent->Left;
        }
        w->Color=x->Parent->Color;
        x->Parent->Color = Black;
        w->Left->Color = Black;
        RotateRight(T,x->Parent);
        x=rootLeft;
      }
    }
  }
  x->Color = Black;
}

Node* Search(Tree* T, Node* X, char* key)
{
  if (X == NULL)
    return NULL;
  while((X != T->NIL) && (strcmp(key,X->key) != 0))
  {
    if (strcmp(key,X->key) < 0)
      X = X->Left;
    else
      X = X->Right;
  }
  return X;
}

Node* Successor(Tree* T, Node* X)
{
  Node* Y;
  if(X->Right != T->NIL)
    return Minimum(T,X->Right);
  Y = X->Parent;
  while(Y != T->NIL && X == Y->Right)
  {
    X = Y;
    Y = Y->Parent;
  }
  return Y;
}

Node* Minimum(Tree* T,Node* X)
{
  while(X->Left != T->NIL)
  {
    X = X->Left;
  }
  return X;
}

