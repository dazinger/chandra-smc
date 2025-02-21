/****************************************************************/
/* Programmer: Zinger                                           */
/*                                                              */
/* tree.h                                                       */
/*                                                              */
/* Code based on Esakov and Weiss, Chapter Seven                */
/****************************************************************/

#ifndef _BINARY_TREE_H
#define _BINARY_TREE_H

#include "globals.h"

ABSTRACT_TYPE(tree);

typedef enum { PREORDER, INORDER, POSTORDER } ORDER;

extern status init_tree    (tree  *p_T);
extern bool   empty_tree   (tree   T);
extern status make_root    (tree  *p_T,            generic_ptr data);
extern void   destroy_tree (tree  *p_T,            void      (*p_func_f)());
extern status traverse_tree(tree   T,              status    (*p_func_f)(),
			    ORDER  order);

extern tree        getleft    (tree T);
extern tree       *getleftadr (tree T);
extern tree        getright   (tree T);
extern tree       *getrightadr(tree T);
extern generic_ptr getdata    (tree T);

#endif
