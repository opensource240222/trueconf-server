/**
 **************************************************************************
 * \file VS_RBTree.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief RedBlack Search Tree interface
 *
 * \b Project Standart Libraries
 * \author Petrovichev
 * \date 27.11.02
 *
 * $Revision: 1 $
 *
 * $History: VS_RBTree.h $
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/std/cpplib
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/std/cpplib
 *
 * *****************  Version 4  *****************
 * User: Stass        Date: 29.04.05   Time: 18:34
 * Updated in $/VS/std/cpplib
 * map wrapper template
 *
 * *****************  Version 3  *****************
 * User: Stass        Date: 15.04.05   Time: 15:28
 * Updated in $/VS/std/cpplib
 * added map template
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 29.12.04   Time: 15:11
 * Updated in $/VS/std/cpplib
 * files headers
 ****************************************************************************/
#ifndef VS_STD_RBTREE_H
#define VS_STD_RBTREE_H

/****************************************************************************
 * Includes
 ****************************************************************************/
#include <cstddef>

/**
 **************************************************************************
 * \brief Red Black Search Tree
 ****************************************************************************/
class VS_RBTree
{
public:
	typedef const void* ConstPointer;
	typedef void* Pointer;
	/// Copy constructor of object
	typedef void* (*Factory) ( const void* x );
	/// Destructor of object
	typedef void (*Destructor) ( void* x );
	// Key relation. Returns 0 if x1 == x2, -1 if x1 < x2, 1 if x1 > x2.
	typedef int (*Predicate) ( const void* x1, const void* x2 );
	struct Node;
	struct Pair
	{
		Pair () : key ( 0 ), data ( 0 ) {}
		Pair ( void* key, void* data ) : key ( key ), data ( data ) {}
		void* key;
		void* data;
	};
	struct Node
	{
		Node* parent;
		Node* left;
		Node* right;
		bool color;			// 0 - red, 1 - black
		Pair pair;
	};

	// Constructors
	VS_RBTree ();
	VS_RBTree ( const VS_RBTree& x );
	~VS_RBTree ();
	VS_RBTree& operator= ( const VS_RBTree& x );

	void SetKeyFactory ( Factory factory, Destructor destructor )
	{
		keyFactory = factory;
		keyDestructor = destructor;
	}
	void SetDataFactory ( Factory factory, Destructor destructor )
	{
		dataFactory = factory;
		dataDestructor = destructor;
	}
	void SetPredicate ( Predicate predicate )
	{	this->predicate = predicate; }
	int KeyCompare ( const void* key1, const void* key2 )
	{	return predicate ( key1, key2 ); }

	// Main operations
	/// Insert new value. Returns new node.
	Node* Insert ( const Pair& x );
	/// Delete node x. Returns node beyond delette.
	Node* Delete ( Node* x );
	/// Assign data to node with key == pair.key.
	/// If the node isn't exist insert it.
	/// Operation is valid for tree with unique keys only.
	Node* Assign ( const Pair& pair );
	/// Assign data to node x.
	void Assign ( Node* x, const void* data );
	/// Find node with key. Else returns Nil.
	Node* Find ( const void* key ) const;
	/// Returns an node that designates the earliest element x
	/// for which Predicate()(x.key, key) is 0 or positive.
	/// In other words, returns first element equal or greater than key.
	/// If no such element exists, the function returns Nil.
	Node* LowerBound ( const void* key ) const;
	/// Returns node with minimum key.
	Node* Minimum ( const Node* n ) const;
	/// Returns node with maximum key.
	Node* Maximum ( const Node* n ) const;
	/// Returns next node in order.
	Node* Next ( const Node* n ) const;
	/// Returns previous node in order.
	Node* Prev ( const Node* n ) const;

	Node* Root () const
	{	return head->right; }
	Node* Nil () const
	{	return head; }
	size_t Size () const
	{	return size; }

public:
	static void* VoidFactory ( const void* x )
	{	return (void*) x; }
	static void VoidDestructor ( void* /*x*/ )
	{	}

	static void* IntFactory ( const void* x )
	{
		int* n = new int;
		*n = *((int*)x);
		return (void*) n; }
	static void IntDestructor ( void* x )
	{
		delete static_cast<int*>(x);
	}
private:
	// Less operator for int
	static int IntPredicate ( const void* x1, const void* x2 )
	{
		if ( x1 == x2 )
			return 0;
		return x1 < x2 ? -1 : 1;
	}
	/// x must have right child y. x will become left child of y,
	/// left child of y will become right child of x
	void LeftRotate ( Node* x );
	/// x must have left child y. x will become right child of y,
	/// right child of y will become left child of x
	void RightRotate ( Node* x );
	void DeleteFixup ( Node* x );
	void InsertFixup ( Node* x );
	void DeleteTree ();
	/// Insert new value to parent node "y" in order. Returns it node.
	Node* InsertTo ( const Pair& pair, Node* y );
	static const bool red;
	static const bool black;
	Node* head;
	size_t size;

public:
	Factory keyFactory;
	Factory dataFactory;
	Destructor keyDestructor;
	Destructor dataDestructor;
	Predicate predicate;
};

#endif	// VS_STD_RBTREE_H
