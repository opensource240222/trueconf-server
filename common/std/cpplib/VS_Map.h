/**
 **************************************************************************
 * \file Map.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief map container interface
 *
 * \b Project Standart Libraries
 * \author V.Morozov
 * \author Petrovichev
 * \date 27.11.02
 *
 * $Revision: 1 $
 *
 * $History: VS_Map.h $
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/std/cpplib
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 5.02.07    Time: 21:40
 * Updated in $/VS2005/std/cpplib
 * - project configuration
 * - depricated functions warnings suppressed
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/std/cpplib
 *
 * *****************  Version 5  *****************
 * User: Stass        Date: 24.05.06   Time: 16:53
 * Updated in $/VS/std/cpplib
 * switched reg server to CI call_id and login
 *
 * *****************  Version 4  *****************
 * User: Stass        Date: 23.11.05   Time: 14:13
 * Updated in $/VS/std/cpplib
 * new maps for registry storage
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
#ifndef VS_STD_MAP_H
#define VS_STD_MAP_H

#include "VS_RBTree.h"
#include "std-generic/clib/strcasecmp.h"

#include <cstdlib>
#include <cstring>
#include <cstdint>

#if defined(_WIN32)
#	define strdup _strdup
#endif

/**
 **************************************************************************
 * \brief Map container class.
 *
 * Key is void*, data is void*.
 * Keys are unique. Set key predicate.
 * Set factory if container must owns data objects and/or keys objects.
 ****************************************************************************/
class VS_Map
{
protected:
	typedef VS_RBTree::Node Node;
public:
	typedef void KeyType;
	typedef VS_RBTree::Pair Pair;
	typedef VS_RBTree::Predicate Predicate;
	typedef VS_RBTree::Factory Factory;
	typedef VS_RBTree::Destructor Destructor;
	typedef const Pair& ConstRef;
	typedef Pair& Reference;
	typedef const Pair* ConstPointer;
	typedef Pair* Pointer;
	typedef const void*& ConstDataRef;
	typedef void*& DataReference;
	class Iterator;
	class ConstIterator
	{
	public:
		ConstIterator () : node ( 0 ), container ( 0 ) {}
		ConstIterator ( const Node* node, const VS_Map* map ) : node ( node ),
			container ( map ) {}
		ConstIterator ( const Iterator& it ) : node ( it.node ),
			container ( it.container ) {}
		ConstRef operator* () const
		{	return node->pair; }
		ConstPointer operator-> () const
		{	return &node->pair; }
		ConstIterator& operator++ ()
		{
			node = container->tree.Next ( node );
			return *this;
		}
		ConstIterator operator++ ( int )
		{
			ConstIterator it ( *this );
			node = container->tree.Next ( node );
			return it;
		}
		ConstIterator& operator-- ()
		{
			node = container->tree.Prev ( node );
			return *this;
		}
		ConstIterator operator-- ( int )
		{
			ConstIterator it ( *this );
			node = container->tree.Prev ( node );
			return it;
		}
		bool operator== ( const ConstIterator& it ) const
		{	return node == it.node; }
		bool operator!= ( const ConstIterator& it ) const
		{	return node != it.node; }
		friend class VS_Map;
	protected:
		const Node* node;
		const VS_Map* container;
	};
	class Iterator
	{
	public:
		Iterator () : node ( 0 ), container ( 0 ) {}
		Iterator ( Node* node, VS_Map* map ) : node ( node ), container ( map ) {}
		virtual ~Iterator() {};
		Reference operator* ()
		{	return node->pair; }
		Pointer operator-> ()
		{	return &node->pair; }
		Iterator& operator++ ()
		{
			node = container->tree.Next ( node );
			return *this;
		}
		Iterator operator++ ( int )
		{
			Iterator it ( *this );
			node = container->tree.Next ( node );
			return it;
		}
		Iterator& operator-- ()
		{
			node = container->tree.Prev ( node );
			return *this;
		}
		Iterator operator-- ( int )
		{
			Iterator it ( *this );
			node = container->tree.Prev ( node );
			return it;
		}
		bool operator== ( const Iterator& it ) const
		{	return node == it.node; }
		bool operator!= ( const Iterator& it ) const
		{	return node != it.node; }
		friend class VS_Map;
		friend class ConstIterator;
	protected:
		Node* node;
		VS_Map* container;
	};
	friend class Iterator;
	friend class ConstIterator;

	// Constructors
	VS_Map () {}
	VS_Map ( const VS_Map& x ) : tree ( x.tree ) {}
	~VS_Map () {}
	VS_Map& operator= ( const VS_Map& x );

	void SetPredicate ( Predicate predicate )
	{	tree.SetPredicate ( predicate ); }
	void SetKeyFactory ( Factory factory, Destructor destructor )
	{	tree.SetKeyFactory ( factory, destructor ); }
	void SetDataFactory ( Factory factory, Destructor destructor )
	{	tree.SetDataFactory ( factory, destructor ); }
	// Capacity
	/// Returns the length of the controlled sequence
	size_t Size () const;
	/// returns true for an empty controlled sequence
	bool Empty () const;

	/// Modifiers
	/// determines whether an element y exists in the sequence whose
	/// key matches that of x. If not, it creates such an element y
	/// and initializes it with x. The function then determines the
	/// iterator it that designates y. If an insertion occurred,
	/// the function returns pair(it, true). Otherwise, it returns
	/// pair(it, false).
	bool Insert ( const Pair& value );
	bool Insert ( const void* key, const void* data );
	/// Also returns iterator to new node if successfully
	bool Insert ( Iterator& it, const Pair& value );
	bool Insert ( Iterator& it, const void* key, const void* data );
	/// calls Erase( begin(), end()).
	void Clear ();
	/// returns an iterator that designates the first element remaining
	/// beyond any elements removed, or end() if no such element exists.
	/// removes the element of the controlled sequence pointed to by it
	Iterator Erase ( Iterator it );
	/// removes the elements in the interval [first, last)
	Iterator Erase ( Iterator first, Iterator last );
	/// removes the elements with sort keys in the range
	/// [lower_bound(key), upper_bound(key)). It returns the number of
	/// elements it removes
	size_t Erase ( const void* key );
	// swaps the controlled sequences between *this and x
	//void Swap ( VS_Map& x );

	// Access functions
	// ConstDataRef operator[] ( const void* key ) const;
	/// Find/Insert value with key. Returns iterator to this value.
	Iterator Assign ( const void* key, const void* data );
	/// Assign data to value of iterator it
	void Assign ( Iterator& it, const void* data );
	/// Returns a iterator that points at the first element
	/// (or just beyond the end of an empty sequence).
	ConstIterator Begin () const
	{	return ConstIterator ( tree.Minimum ( tree.Root()), this ); }
	Iterator Begin ()
	{	return Iterator ( tree.Minimum ( tree.Root()), this ); }
	/// returns a iterator that points just beyond the end
	/// of the sequence
	ConstIterator End () const
	{	return ConstIterator ( tree.Nil(), this ); }
	Iterator End ()
	{	return Iterator ( tree.Nil(), this ); }
	ConstIterator Next ( ConstIterator it ) const
	{	return ConstIterator ( tree.Next ( it.node ), this ); }
	Iterator Next ( Iterator it )
	{	return Iterator ( tree.Next ( it.node ), this ); }
	ConstIterator Prev ( ConstIterator it ) const
	{	return ConstIterator ( tree.Next ( it.node ), this ); }
	Iterator Prev ( Iterator it )
	{	return Iterator ( tree.Prev ( it.node ), this ); }

	/// returns an iterator that designates the earliest element
	/// in the controlled sequence whose sort key equals key.
	/// If no such element exists, the iterator equals end().
	Iterator Find ( const void* key );
	ConstIterator Find ( const void* key ) const;
	/// returns an iterator that designates the earliest element x
	/// in the controlled sequence for which key_comp()(x.key, key) is false.
	/// If no such element exists, the function returns end().
	Iterator LowerBound ( const void* key );
	ConstIterator LowerBound ( const void* key ) const;
	/// returns an iterator that designates the earliest element x
	/// in the controlled sequence for which key_comp()(key, x.first) is true.
	/// If no such element exists, the function returns end().
	Iterator UpperBound ( const void* key );
	ConstIterator UpperBound ( const void* key ) const;
	/// returns a pair of iterators such that lower.key == lower_bound(key)
	/// and upper.key == upper_bound(key).
	void EqualRange ( const void* key, ConstIterator& lower,
		ConstIterator& upper ) const;
	void EqualRange ( const void* key, ConstIterator& lower,
		ConstIterator& upper );
	/// returns the number of elements x in the range
	/// [lower_bound(key), upper_bound(key))
	size_t Count ( const void* key ) const;

	///	test members
	bool IsValid () const;

  /// default predicates
 static inline void* StrFactory ( const void* str ) {
		if (!str)
			return nullptr;
		return strdup(static_cast<const char*>(str));
	}
	static inline void StrDestructor ( void* str )	{	if (str) free(str); }
	static inline int StrPredicate ( const void* str1, const void* str2 )	{
		return strcmp(static_cast<const char*>(str1), static_cast<const char*>(str2));
	}
	static inline int StrIPredicate ( const void* str1, const void* str2 )	{
		return strcasecmp(static_cast<const char*>(str1), static_cast<const char*>(str2));
	}

	static inline void* Int32Factory ( const void* x) { return new uint32_t(*static_cast<const uint32_t*>(x)); }
	static inline void Int32Destructor ( void* x )    { delete static_cast<const uint32_t*>(x);}
	static inline int Int32Predicate ( const void* x1, const void* x2 )
	{
		auto int1 = *static_cast<const uint32_t*>(x1);
		auto int2 = *static_cast<const uint32_t*>(x2);
		return int1 < int2 ? -1 : int2 < int1 ? 1 : 0;
	}

	static inline void* Int64Factory ( const void* x) { return new uint64_t(*static_cast<const uint64_t*>(x)); }
	static inline void Int64Destructor ( void* x )    { delete static_cast<const uint64_t*>(x);}
	static inline int Int64Predicate ( const void* x1, const void* x2 )
	{
		auto int1 = *static_cast<const uint64_t*>(x1);
		auto int2 = *static_cast<const uint64_t*>(x2);
		return int1 < int2 ? -1 : int2 < int1 ? 1 : 0;
	}

protected:
	VS_RBTree tree;
};

#endif // VS_STD_MAP_H
