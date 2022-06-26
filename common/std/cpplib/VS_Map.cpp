/**
 **************************************************************************
 * \file Map.cpp
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief implemantation of Map container
 *
 * \b Project Standart Libraries
 * \author V.Morozov
 * \author Petrovichev
 * \date 27.11.02
 *
 * $Revision: 1 $
 *
 * $History: VS_Map.cpp $
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
#include "VS_Map.h"

VS_Map& VS_Map::operator= ( const VS_Map& x )
{
	if ( this != &x )
	{
		tree = x.tree;
	}
	return *this;
}

// Capacity
// Returns the length of the controlled sequence
size_t VS_Map::Size () const
{
	return tree.Size();
}

// returns true for an empty controlled sequence
bool VS_Map::Empty () const
{
	return tree.Size() ? false : true;
}

// Modifiers
// determines whether an element y exists in the sequence whose
// key matches that of x. If not, it creates such an element y
// and initializes it with x. The function then determines the
// iterator it that designates y. If an insertion occurred,
// the function returns pair(it, true). Otherwise, it returns
// pair(it, false).
bool VS_Map::Insert ( const Pair& value )
{
	Node* i = tree.Find ( value.key );
	bool result = true;
	if ( i == tree.Nil())
		i = tree.Insert ( value );
	else
		result = false;
	return result;
}

bool VS_Map::Insert ( const void* key, const void* data )
{
	Pair value ((void*) key, (void*) data );
	return Insert ( value );
}

// Also returns iterator to new node if successfully
bool VS_Map::Insert ( Iterator& it, const Pair& value )
{
	Node* i = tree.Find ( value.key );
	bool result = true;
	if ( i == tree.Nil())
		i = tree.Insert ( value );
	else
		result = false;
	it.container = this;
	it.node = i;
	return result;
}

bool VS_Map::Insert ( Iterator& it, const void* key, const void* data )
{
	Pair value ((void*) key, (void*) data );
	return Insert ( it, value );
}

// calls Erase( begin(), end()).
void VS_Map::Clear ()
{
	Erase ( Begin(), End());
}

// returns an iterator that designates the first element remaining
// beyond any elements removed, or end() if no such element exists.
// removes the element of the controlled sequence pointed to by it
VS_Map::Iterator VS_Map::Erase ( Iterator it )
{
	Node* n = tree.Delete ( it.node );
	return Iterator ( n, this );
}

// removes the elements in the interval [first, last)
VS_Map::Iterator VS_Map::Erase ( Iterator first, Iterator last )
{
	Node* i;
	for (i = first.node; i != last.node; )
		i = tree.Delete ( i );
	return Iterator ( i, this );
}

// removes the elements with sort keys in the range
// [lower_bound(key), upper_bound(key)). It returns the number of
// elements it removes
size_t VS_Map::Erase ( const void* key )
{
	size_t result = 0;
	Node* i = tree.Find ( key );
	if ( i != tree.Nil())
	{
		tree.Delete ( i );
		++result;
	}
	return result;
}

VS_Map::Iterator VS_Map::Assign ( const void* key, const void* data )
{
	Node* i = tree.Assign ( Pair ( (void*) key, (void*) data ));
	return Iterator ( i, this );
}

// Assign data to value of iterator it
void VS_Map::Assign ( Iterator& it, const void* data )
{
	tree.Assign ( it.node, data );
}

// Find
// returns an iterator that designates the earliest element
// in the controlled sequence whose sort key equals key.
// If no such element exists, the iterator equals end().
VS_Map::Iterator VS_Map::Find ( const void* key )
{
	Node* i = tree.Find ( key );
	return Iterator ( i, this );
}

VS_Map::ConstIterator VS_Map::Find ( const void* key ) const
{
	Node* i = tree.Find ( key );
	return ConstIterator ( i, this );
}

// returns an iterator that designates the earliest element x
// in the controlled sequence for which key_comp()(x.key, key) is false.
// If no such element exists, the function returns end().
VS_Map::Iterator VS_Map::LowerBound ( const void* key )
{
	return Iterator ( tree.LowerBound ( key ), this );
}

VS_Map::ConstIterator VS_Map::LowerBound ( const void* key ) const
{
	return ConstIterator ( tree.LowerBound ( key ), this );
}

// returns an iterator that designates the earliest element x
// in the controlled sequence for which key_comp()(key, x.first) is true.
// If no such element exists, the function returns end().
VS_Map::Iterator VS_Map::UpperBound ( const void* key )
{
  Iterator it;
  return ((it=Find (key))==End())?End():Prev(it);
}
VS_Map::ConstIterator VS_Map::UpperBound ( const void* key ) const
{
  ConstIterator cit;
  return ((cit=Find (key))==End())?End():Prev(cit);
}

// returns a pair of iterators such that lower.key == lower_bound(key)
// and upper.key == upper_bound(key).
void VS_Map::EqualRange ( const void* key, ConstIterator& lower,
	ConstIterator& upper ) const
{
  lower = LowerBound(key);
  upper = UpperBound(key);
}
void VS_Map::EqualRange ( const void* key, ConstIterator& lower,
	ConstIterator& upper )
{
  lower = LowerBound(key);
  upper = UpperBound(key);
}
// returns the number of elements x in the range
// [lower_bound(key), upper_bound(key))
size_t VS_Map::Count ( const void* key ) const
{
  return (Find(key)==End())?0:1;
}

//	test members
bool VS_Map::IsValid () const
{
	return true;
}

