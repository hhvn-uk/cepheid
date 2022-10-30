#include <stdlib.h>
#include "main.h"

/* This interface is purposely limited to adding children, as opposed to
 * appending data on the same level. This is done on purpose, so that ->u
 * can always be set. It also means there is always a root Tree structure,
 * which does not need to be created with malloc and friends. */
Tree *
tree_add_child(Tree *t, char *name, int type, void *data, Tree **ptr) {
	Tree *p;
	Tree *e;

	if (!t) return NULL;

	e = malloc(sizeof(Tree));
	if (!e) return NULL;

	e->p = e->n = e->d = NULL;
	e->name = name;
	e->type = type;
	e->data = data;
	e->u = t;
	e->collapsed = 0;

	if (!t->d) {
		t->d = e;
	} else {
		for (p = t->d; p->n; p = p->n);
		p->n = e;
		e->p = p;
	}

	if (ptr)
		*ptr = e;

	return e;
}

/* Deletes a node in the tree and replaces it with its children. */
int
tree_delete(Tree **t, int freedata) {
	Tree *e;
	Tree *p;
	Tree *fc, *lc;

	if (!*t)
		return -1;

	e = *t;

	if (!e->u)
		warning("trying to delete root of tree\n");

	if (e->d) {
		fc = e->d;
		for (p = fc; p->n; p = p->n);
		lc = p;
	} else {
		fc = lc = NULL;
	}

	if (e->p)
		e->p->n = fc ? fc : e->n;
	else
		e->u->d = e->n;

	if (e->n)
		e->n->p = lc ? lc : e->p;

	if (freedata)
		free(e->data);
	free(e);

	return 0;
}

static Tree *
tree_delete_r_sub(Tree *e, int freedata) {
	Tree *c;
	Tree *n;

	c = e->d;
	n = e->n;

	if (freedata)
		free(e->data);
	free(e);

	while (c)
		c = tree_delete_r_sub(c, freedata);

	return n;
}

/* Deletes a node and its children. */
int
tree_delete_r(Tree **t, int freedata) {
	Tree *e;

	if (!*t)
		return -1;

	e = *t;

	if (!e->u)
		warning("trying to delete root of tree\n");

	if (e->p)
		e->p->n = e->n;
	else
		e->u->d = e->n;

	if (e->n)
		e->n->p = e->p;

	tree_delete_r_sub(e, freedata);
}

int
tree_iter(Tree *t, int maxdepth, Tree **p, int *depth) {
	if (!*p) {
		*p = t->d;
		if (depth) *depth = 0;
		return *p ? 0 : -1;
	}

	if ((maxdepth < 0 || *depth <= maxdepth) && (*p)->d) {
		*p = (*p)->d;
		if (depth) (*depth)++;
		return 0;
	}

	if ((*p)->n) {
		*p = (*p)->n;
		return 0;
	}

	for (; *p && *p != t; *p = (*p)->u) {
		if ((*p)->n) {
			*p = (*p)->n;
			return 0;
		}
		if (depth) (*depth)--;
	}

	return -1;
}
