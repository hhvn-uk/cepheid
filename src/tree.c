#include <stdlib.h>
#include "main.h"

/* In cepheid, the Tree is a data structure that hierarchically represents data
 * of arbitrary types. It is in essence a quadrupally linked list.
 *
 * Each tree should enumerate the types (not necessarily a typedef) of data it
 * stores. These enums must only have 1 bit set to 1.
 *
 * The Treeview gui element displays a tree graphically.
 * selmask and colmask are set such that only nodes with a ->type that matches
 * the mask will be: selectable or collapsible, respectively.
 */

/* This interface is purposely limited to adding children, as opposed to
 * appending data on the same level. This is done on purpose, so that ->u
 * can always be set. It also means there is always a root Tree structure,
 * which does not need to be created with malloc and friends. */
Tree *
tree_add_child(Tree *t, char *name, int type, void *data, Tree **ptr) {
	Tree *p;
	Tree *e;

	if (!t) return NULL;

	e = emalloc(sizeof(Tree));
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
tree_delete(Tree **t, Treefree freedata) {
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
		freedata(e);
	free(e);

	*t = NULL;

	return 0;
}

static Tree *
tree_delete_r_sub(Tree *e, Treefree freedata) {
	Tree *c;
	Tree *n;

	c = e->d;
	n = e->n;

	if (freedata)
		freedata(e);
	free(e);

	while (c)
		c = tree_delete_r_sub(c, freedata);

	return n;
}

/* Deletes a node and its children. */
int
tree_delete_r(Tree **t, Treefree freedata) {
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

	*t = NULL;

	return 0;
}

int
tree_delete_root(Tree *t, Treefree freedata) {
	return tree_delete_r(&t->d, freedata);
}

static int
tree_iter_f_yes(Tree *t, void *data) {
	return 1;
}

#define TREPORT(direction) printf("%d %s | Direction: %s\n", *depth, (*p)->name, direction)

int
tree_iter_f(Tree *t, int maxdepth, Tree **p, int *depth,
		Treefilter filter, void *fdata) {
	if (!filter)
		filter = tree_iter_f_yes;

	if (!*p) {
		*p = t->d;
		if (depth) *depth = 0;
		return *p ? 0 : -1;
	}

skip:
	if ((maxdepth < 0 || *depth <= maxdepth) && (*p)->d) {
		*p = (*p)->d;
		if (depth) (*depth)++;
		if (!filter(*p, fdata))
			goto skip;
		return 0;
	}

	if ((*p)->n) {
		*p = (*p)->n;
		if (!filter(*p, fdata))
			goto skip;
		return 0;
	}

	for (; *p && *p != t; *p = (*p)->u) {
		if ((*p)->n) {
			*p = (*p)->n;
			if (!filter(*p, fdata))
				goto skip;
			return 0;
		}
		if (depth) (*depth)--;
	}

	return -1;
}

int
tree_iter(Tree *t, int maxdepth, Tree **p, int *depth) {
	return tree_iter_f(t, maxdepth, p, depth, NULL, NULL);
}

void
tree_sort_sideways(Tree *t, Treecompar compar, void *cdata) {
	Tree *cp, *np, *p;

	for (; t->p; t = t->p);

	cp = t->n;
	while (cp) {
		np = cp->n;

		p = cp->p;

		if (cp->p) cp->p->n = cp->n;
		if (cp->n) cp->n->p = cp->p;

		for (; p; p = p->p) {
			if (compar(p, cp, cdata) < 0) {
				if (p->n) p->n->p = cp;
				cp->n = p->n;
				cp->p = p;
				p->n = cp;
				break;
			}
			if (!p->p) {
				t->u->d = cp;
				cp->p = NULL;
				cp->n = p;
				p->p = cp;
				break;
			}
		}

		cp = np;
	}
}

static int
tree_sort_filter(Tree *t, void *data) {
	return !t->p && t->u;
}

void
tree_sort(Tree *t, Treecompar compar, void *cdata) {
	Tree *p;
	int depth;
	Tree **s;
	int sl, si;

	/* The tree can't be sorted (i,e, modified) whilst iterating through it. */
	sl = 10;
	s = emalloc(sl * sizeof(Tree *));

	for (p = NULL, si = 0; tree_iter_f(t, TREEMAX, &p, &depth, tree_sort_filter, NULL) != -1; si++) {
		if (si == sl - 1) {
			sl += 10;
			s = erealloc(s, sl * sizeof(Tree *));
		}

		s[si] = p;
	}

	while (si--)
		tree_sort_sideways(s[si], compar, cdata);

	free(s);
}
