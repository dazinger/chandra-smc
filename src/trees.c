/****************************************************************/
/* Programmer: Zinger                                           */
/*                                                              */
/* tree.c                                                       */
/*                                                              */
/* Code based on Esakov and Weiss, Chapter Seven                */
/****************************************************************/

#include "globals.h"
#include "trees.h"
#include <stdlib.h>

typedef struct tree_node *tree_rep;

typedef struct tree_node {
	generic_ptr datapointer;
        tree_rep left;
	tree_rep right;
} tree_node;

#define DATA(tree_rep)     ((tree_rep)->datapointer)
#define LEFT(tree_rep)     ((tree_rep)->left)
#define RIGHT(tree_rep)    ((tree_rep)->right)
#define LEFTADR(tree_rep)  (&((tree_rep)->left))
#define RIGHTADR(tree_rep) (&((tree_rep)->right))

#define TOTREE(tree_rep)     ((tree)         (tree_rep))
#define TOREP(tree)          ((tree_rep)     (tree))

#define TOTREEPTR(tree_rep_ptr) ((tree *)     (tree_rep_ptr))
#define TOREPPTR(tree_ptr)      ((tree_rep *)(tree_ptr))

static status allocate_tree_node(tree_rep *p_T, generic_ptr data)
{
	/*
	 *  Allocate a tree_node and initialize the DATA field with data.
	 */
        tree_rep T = (tree_rep) malloc(sizeof(tree_node));

	if (T == NULL)
		return ERROR;

	*p_T = T;
	DATA(T) = data;
	LEFT(T) = NULL;
	RIGHT(T) = NULL;
	return OK;
}

static void free_tree_node(tree *p_T)
{
	/*
	 *  Reclaim the space used by the tree_node.
	 */
	free(*p_T);
	*p_T = NULL;
}

status init_tree(tree *p_T)
{
	/*
	 *  Initialize a tree to empty.
	 */
	*p_T = NULL;
	return OK;
}

bool empty_tree(tree T)
{
	/*
	 *  Return TRUE if T is empty, otherwise return FALSE.
	 */
	return (T == NULL) ? TRUE : FALSE;
}

status make_root(tree *p_T, generic_ptr data)
{
	/*
	 *  Allocate a new node and make that the root of a tree.
	 */
	tree_rep *tree_rep_ptr = TOREPPTR(p_T);

	if (empty_tree(*p_T) == FALSE)
		return ERROR;

	if (allocate_tree_node(tree_rep_ptr, data) == ERROR)
		return ERROR;

	init_tree(TOTREEPTR(LEFTADR(TOREP(*tree_rep_ptr))));
	init_tree(TOTREEPTR(RIGHTADR(TOREP(*tree_rep_ptr))));

	p_T = TOTREEPTR(tree_rep_ptr);

	return OK;
}

void destroy_tree(tree *p_T, void(*p_func_f)())
{
	/*
	 *  Delete an entire tree (uses a postorder traversal), calling
	 *  p_func_f with the DATA stored at each node.
	 */
	tree_rep *tree_rep_ptr = TOREPPTR(p_T);

	if (empty_tree(*p_T) == FALSE) {
		destroy_tree(TOTREE(LEFT(*tree_rep_ptr)), p_func_f);
		destroy_tree(TOTREE(RIGHT(*tree_rep_ptr)), p_func_f);
		if (p_func_f != NULL)
			(*p_func_f)(DATA(*tree_rep_ptr));
		free_tree_node(p_T);
	}
}

static status preorder_traverse(tree T, status (*p_func_f)())
{
        /*
         *  Traverse a tree in preorder, calling p_func_f() with the
         *  DATA stored in each node visited.
         */
        tree_rep t_rep = TOTREE(T);
        status rc;

        if (empty_tree(T) == TRUE)
                return OK;
        rc = (*p_func_f)(DATA(t_rep));
        if (rc == OK)
                rc = preorder_traverse(TOTREE(LEFT(t_rep)), p_func_f);
        if (rc == OK)
                rc = preorder_traverse(TOTREE(RIGHT(t_rep)), p_func_f);
        return rc;
}

static status inorder_traverse(tree T, status (*p_func_f)())
{
        /*
         *  Traverse a tree in inorder, calling p_func_f() with the
         *  DATA stored in each node visited.
         */
        tree_rep t_rep = TOTREE(T);
        status rc;

        if (empty_tree(T) == TRUE)
                return OK;
        rc = inorder_traverse(TOTREE(LEFT(t_rep)), p_func_f);
        if (rc == OK)
                rc = (*p_func_f)(DATA(t_rep));
        if (rc == OK)
                rc = inorder_traverse(TOTREE(RIGHT(t_rep)), p_func_f);
        return rc;
}

static status postorder_traverse(tree T, status (*p_func_f)())
{
        /*
         *  Traverse a tree in post-order calling p_func_f() with the
         *  DATA stored in each node visited.
         */
        tree_rep t_rep = TOTREE(T);
        status rc;

        if (empty_tree(T) == TRUE)
                return OK;
        rc = postorder_traverse(TOTREE(LEFT(t_rep)), p_func_f);
        if (rc == OK)
                rc = postorder_traverse(TOTREE(RIGHT(t_rep)), p_func_f);
        if (rc == OK)
                rc = (*p_func_f)(DATA(t_rep));
        return rc;
}

status traverse_tree(tree T,          status (*p_func_f)(), ORDER    order)
{
	/*
	 *  Traverse a tree in preorder, postorder, or inorder.
	 */
	switch (order) {
	case PREORDER:
		return preorder_traverse(T, p_func_f);
	case INORDER:
		return inorder_traverse(T, p_func_f);
	case POSTORDER:
		return postorder_traverse(T, p_func_f);
	}
	return ERROR;
}

tree getleft(tree T)
{
	tree_rep tree_local;
	tree_local = (tree_rep) T;

	return TOTREE(LEFT(tree_local));
}

tree *getleftadr(tree T)
{
        tree_rep tree_local;
        tree_local = (tree_rep) T;

        return TOTREEPTR(LEFTADR(tree_local));
}

tree getright(tree T)
{
        tree_rep tree_local;
        tree_local = (tree_rep) T;

        return TOTREE(RIGHT(tree_local));
}

tree *getrightadr(tree T)
{
        tree_rep tree_local;
        tree_local = (tree_rep) T;

        return TOTREEPTR(RIGHTADR(tree_local));
}

generic_ptr getdata(tree T)
{
        tree_rep tree_local;
        tree_local = (tree_rep) T;

        return DATA(tree_local);
}
