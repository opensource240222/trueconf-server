/**
 **************************************************************************
 * \file VS_RBTree.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief Red Black Search Tree implementation
 *
 * \b Project Standart Libraries
 * \author Petrovichev
 * \author Description at T.Cormen "Introduction to Algorithms" pages 255-266
 * \date 27.11.02
 *
 * $Revision: 1 $
 *
 * $History: VS_RBTree.cpp $
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/std/cpplib
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/std/cpplib
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 29.12.04   Time: 15:11
 * Updated in $/VS/std/cpplib
 * files headers
 ****************************************************************************/


/****************************************************************************
 * Includes
 ****************************************************************************/
#include "VS_RBTree.h"

const bool VS_RBTree::red = false;
const bool VS_RBTree::black = true;

VS_RBTree::VS_RBTree ()
{
	keyFactory = VoidFactory;
	dataFactory = VoidFactory;
	predicate = IntPredicate;
	keyDestructor = 0;
	dataDestructor = 0;
	head = new Node;
	head->parent = head;
	head->left = head;
	head->right = head;
	head->color = black;
	head->pair.key = 0;
	head->pair.data = 0;
	size = 0;
}

VS_RBTree::VS_RBTree ( const VS_RBTree& x )
{
	head = new Node;
	head->parent = head;
	head->left = head;
	head->right = head;
	head->color = black;
	head->pair.key = 0;
	head->pair.data = 0;
	head->right = head;
	keyFactory = x.keyFactory;
	dataFactory = x.dataFactory;
	keyDestructor = x.keyDestructor;
	dataDestructor = x.dataDestructor;
	predicate = x.predicate;
	for ( Node* i = x.Minimum(x.head->right); i != x.head; i = x.Next ( i ))
		Insert ( i->pair );
	size = x.size;
}

VS_RBTree::~VS_RBTree ()
{
	DeleteTree();
	delete head;
}

void VS_RBTree::DeleteTree ()
{
	for ( Node* i = head->right; i != head; )
	{
		if ( i->left != head )
		{
			i = i->left;
			continue;
		}
		if ( i->right != head )
		{
			i = i->right;
			continue;
		}
		// no childs
		if ( keyFactory != VoidFactory )
			keyDestructor ( i->pair.key );
		if ( dataFactory != VoidFactory )
			dataDestructor ( i->pair.data );
		Node* d = i;
		i = i->parent;
		if ( i->left == d )
			i->left = head;
		else
			i->right = head;
		delete d;
	}
	size = 0;
}

VS_RBTree& VS_RBTree::operator= ( const VS_RBTree& x )
{
	if ( this != &x )
	{
		DeleteTree();
		head->right = head;
		keyFactory = x.keyFactory;
		dataFactory = x.dataFactory;
		keyDestructor = x.keyDestructor;
		dataDestructor = x.dataDestructor;
		predicate = x.predicate;
		for ( Node* i = x.Minimum(x.head->right); i != x.head; i = x.Next ( i ))
			Insert ( i->pair );
		size = x.size;
	}
	return *this;
}

// Insert new value. Returns it node.
VS_RBTree::Node* VS_RBTree::Insert ( const Pair& pair )
{
	// find fit place at tree down
	Node* y = head;
	for ( Node* x = head->right; x != head; )
	{
		y = x;
		x = predicate ( pair.key, x->pair.key ) < 0 ? x->left : x->right;
	}
	// insert node to down of tree
	return InsertTo ( pair, y );
}

// Insert new value to parent node "y" in order. Returns it node.
VS_RBTree::Node* VS_RBTree::InsertTo ( const Pair& pair, Node* y )
{
	// insert node as child of node y ( to down of tree )
	Node* z = new Node;
	z->left = head;
	z->right = head;
	z->pair.key = keyFactory ( pair.key );
	z->pair.data = dataFactory ( pair.data );
	z->parent = y;
	if ( y == head )
		y->right = z;
	else
	{
		if ( predicate ( z->pair.key, y->pair.key ) < 0 )
			y->left = z;
		else
			y->right = z;
	}
	// balancing
	z->color = red;
	InsertFixup ( z );
	++size;
	return z;
}

void VS_RBTree::InsertFixup ( Node* z )
{
	Node* y;
	for ( Node* x = z; x != head->right && x->parent->color == red; )
	{
		Node* grandfather = x->parent->parent;
		if ( x->parent == grandfather->left )
		{
			y = grandfather->right;
			if ( y->color == red )
			{	// red
				x->parent->color = black;
				y->color = black;
				grandfather->color = red;
				x = grandfather;
			}
			else
			{	// black
				if ( x == x->parent->right )
				{	// right child
					x = x->parent;
					LeftRotate ( x );
				}
				x->parent->color = black;
				x->parent->parent->color = red;
				RightRotate ( x->parent->parent );
			}
		}
		else
		{
			y = grandfather->left;
			if ( y->color == red )
			{	// red
				x->parent->color = black;
				y->color = black;
				grandfather->color = red;
				x = grandfather;
			}
			else
			{	// black
				if ( x == x->parent->left )
				{	// left child
					x = x->parent;
					RightRotate ( x );
				}
				x->parent->color = black;
				x->parent->parent->color = red;
				LeftRotate ( x->parent->parent );
			}
		}
	}
	head->right->color = black;
}

// Delete node x. Returns node beyond delete.
VS_RBTree::Node* VS_RBTree::Delete ( Node* z )
{
	Node* r = Next ( z );	// returning successor of delete node
	Node* y;		// delete
	if ( z->left == head || z->right == head )
		y = z;
	else
	{	// has both childs, delete successor node
		y = r;
		r = z;
	}
	Node* x;		// single child of delete
	// x become child of y's parent
	if ( y->left != head )
		x = y->left;
	else
		x = y->right;
	x->parent = y->parent;
	if ( y == y->parent->left )
		y->parent->left = x;
	else
		y->parent->right = x;
	if ( y != z )
	{	// copy value from y to z
		if ( keyFactory != VoidFactory )
			keyDestructor ( z->pair.key );
		if ( dataFactory != VoidFactory )
			dataDestructor ( z->pair.data );
		z->pair.key = y->pair.key;		// factory don't need
		z->pair.data = y->pair.data;	// factory don't need
	}
	if ( y->color == black )
		DeleteFixup ( x );
	head->parent = head;
	// delete node y from memory
	if ( y == z )
	{
		if ( keyFactory != VoidFactory )
			keyDestructor ( y->pair.key );
		if ( dataFactory != VoidFactory )
			dataDestructor ( y->pair.data );
	}
	delete y;
	--size;
	return r;
}

void VS_RBTree::DeleteFixup ( Node* x )
{
	for ( ; x != head->right && x->color == black; )
	{
		if ( x == x->parent->left )
		{	// left child
			Node* w = x->parent->right;	// brother of x
			if ( w->color == red )
			{	// case 1st
				w->color = black;
				x->parent->color = red;
				LeftRotate ( x->parent );
				w = x->parent->right;
			}
			if ( w->left->color == black && w->right->color == black )
			{	// case 2nd
				w->color = red;
				x = x->parent;
			}
			else
			{
				if ( w->right->color == black )
				{	// case 3th
					w->left->color = black;
					w->color = red;
					RightRotate ( w );
					w = x->parent->right;
				}
				// case 4th
				w->color = x->parent->color;
				x->parent->color = black;
				w->right->color = black;
				LeftRotate ( x->parent );
				x = head->right;
			}
		}
		else
		{	// right child
			Node* w = x->parent->left;	// brother of x
			if ( w->color == red )
			{	// case 1st
				w->color = black;
				x->parent->color = red;
				RightRotate ( x->parent );
				w = x->parent->left;
			}
			if ( w->left->color == black && w->right->color == black )
			{	// case 2nd
				w->color = red;
				x = x->parent;
			}
			else
			{
				if ( w->left->color == black )
				{	// case 3th
					w->right->color = black;
					w->color = red;
					LeftRotate ( w );
					w = x->parent->left;
				}
				// case 4th
				w->color = x->parent->color;
				x->parent->color = black;
				w->left->color = black;
				RightRotate ( x->parent );
				x = head->right;
			}
		}
	}
	x->color = black;
}

// Find node with key. Else returns Nil.
VS_RBTree::Node* VS_RBTree::Find ( const void* key ) const
{
	int compare;
	Node* n;
	for ( n = head->right; n != head &&
		( compare = predicate ( key, n->pair.key )) ? true : false; )
		n = compare < 0 ? n->left : n->right;
	return n;
}

// Returns an node that designates the earliest element x
// for which Predicate()(x.key, key) is 0 or positive.
// In other words, returns first element equal or greater than key.
// If no such element exists, the function returns Nil.
VS_RBTree::Node* VS_RBTree::LowerBound ( const void* key ) const
{
	int compare = (-1);
	Node* n, * p;
	for ( p = n = head->right; n != head &&
		( compare = predicate ( key, n->pair.key )) ? true : false; )
	{
		p = n;
		n = compare < 0 ? n->left : n->right;
	}
	if ( n != head )	// found node with the key
		return n;
	// not found node with the key
	if ( compare < 0 )
		return p;
	return Next ( p );
}

// Assign data to node with key == pair.key. If the node isn't exist insert it.
// Valid only for tree with unique keys.
VS_RBTree::Node* VS_RBTree::Assign ( const Pair& pair )
{
	// Find node with equal key
	int compare;
	Node* n, * p;
	for ( p = n = head->right; n != head &&
		( compare = predicate ( pair.key, n->pair.key )) ? true : false; )
	{
		p = n;
		n = compare < 0 ? n->left : n->right;
	}
	if ( n != head )	// found node with the key
	{
		if ( dataFactory != VoidFactory )
			dataDestructor ( n->pair.data );
		n->pair.data = dataFactory ( pair.data );
		return n;
	}
	else				// not found node with the key
		return InsertTo ( pair, p );
}

// Assign data to node x.
void VS_RBTree::Assign ( Node* x, const void* data )
{
	if ( x != head )
	{
		if ( dataFactory != VoidFactory )
			dataDestructor ( x->pair.data );
		x->pair.data = dataFactory ( data );
	}
}

// Returns node with minimum key.
VS_RBTree::Node* VS_RBTree::Minimum ( const Node* n ) const
{
	const Node* p;
	for ( p = n; n != head; p = n, n = n->left );
	return (Node*) p;
}

// Returns node with maximum key.
VS_RBTree::Node* VS_RBTree::Maximum ( const Node* n ) const
{
	const Node* p;
	for ( p = n; n != head; p = n, n = n->right );
	return (Node*) p;
}

// Returns next node in order.
VS_RBTree::Node* VS_RBTree::Next ( const Node* n ) const
{
	if ( n->right != head )
		return Minimum ( n->right );
	Node *i, *p;
	for ( i = n->parent, p = (Node*) n; i != head && p == i->right;
		p = i, i = i->parent );
	return i;
}

// Returns previous node in order.
VS_RBTree::Node* VS_RBTree::Prev ( const Node* n ) const
{
	if ( n->left != head )
		return Maximum ( n->left );

	Node *i,*p;
	for (i = n->parent, p = (Node*) n; i != head && p == i->left;
		p = i, i = i->parent );
	return i;
}

// x must have right child y. x will become left child of y,
// left child of y will become right child of x
void VS_RBTree::LeftRotate ( Node* x )
{
	Node* y = x->right;
	if ( y == head )
		return;
	x->right = y->left;
	if ( y->left != head )
		y->left->parent = x;
	y->parent = x->parent;
	if ( x == x->parent->left )
		x->parent->left = y;
	else
		x->parent->right = y;
	y->left = x;
	x->parent = y;
}

// x must have left child y. x will become right child of y,
// right child of y will become left child of x
void VS_RBTree::RightRotate ( Node* x )
{
	Node* y = x->left;
	if ( y == head )
		return;
	x->left = y->right;
	if ( y->right != head )
		y->right->parent = x;
	y->parent = x->parent;
	if ( x == x->parent->left )
		x->parent->left = y;
	else
		x->parent->right = y;
	y->right = x;
	x->parent = y;
}
