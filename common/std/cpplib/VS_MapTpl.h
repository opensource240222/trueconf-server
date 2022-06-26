/**
 **************************************************************************
 * \file VS_MapTpl.cpp
 * (c) 2005 Visicron Inc.  http://www.visicron.net/
 * \brief interlocked variable
 *
 * $Revision: 2 $
 *
 * $History: VS_MapTpl.h $
 *
 * *****************  Version 2  *****************
 * User: Stass        Date: 12.02.08   Time: 20:31
 * Updated in $/VSNA/std/cpplib
 * passing aliases to as
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/std/cpplib
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/std/cpplib
 *
 * *****************  Version 11  *****************
 * User: Stass        Date: 24.05.06   Time: 16:53
 * Updated in $/VS/std/cpplib
 * switched reg server to CI call_id and login
 *
 * *****************  Version 10  *****************
 * User: Stass        Date: 23.11.05   Time: 14:13
 * Updated in $/VS/std/cpplib
 * new maps for registry storage
 *
 * *****************  Version 9  *****************
 * User: Stass        Date: 21.11.05   Time: 17:55
 * Updated in $/VS/std/cpplib
 * multi user support
 *
 * *****************  Version 8  *****************
 * User: Stass        Date: 31.05.05   Time: 15:44
 * Updated in $/VS/std/cpplib
 * added accessor func
 *
 * *****************  Version 7  *****************
 * User: Stass        Date: 19.05.05   Time: 14:03
 * Updated in $/VS/std/cpplib
 * added one load
 *
 * *****************  Version 6  *****************
 * User: Stass        Date: 14.05.05   Time: 18:12
 * Updated in $/VS/std/cpplib
 * const fix
 *
 * *****************  Version 5  *****************
 * User: Stass        Date: 14.05.05   Time: 18:01
 * Updated in $/VS/std/cpplib
 * renamed resolve
 *
 * *****************  Version 4  *****************
 * User: Stass        Date: 5.05.05    Time: 19:45
 * Updated in $/VS/std/cpplib
 *
 * *****************  Version 3  *****************
 * User: Stass        Date: 4.05.05    Time: 15:09
 * Updated in $/VS/std/cpplib
 * changed "magic key" logic
 *
 * *****************  Version 2  *****************
 * User: Stass        Date: 29.04.05   Time: 18:34
 * Updated in $/VS/std/cpplib
 * map wrapper template
 *
 * *****************  Version 1  *****************
 * User: Stass        Date: 15.04.05   Time: 15:28
 * Created in $/VS/std/cpplib
 * added map template
 *
 ****************************************************************************/

#ifndef VS_MAP_TEMPLATE
#define VS_MAP_TEMPLATE

#include "VS_Map.h"

template<class T>
int TDefPredicate ( const void* x1, const void* x2 )
{
	 auto p1 = static_cast<const T*>(x1);
	 auto p2 = static_cast<const T*>(x2);
	 return *p1 == *p2 ? 0 : *p1 < *p2 ? -1 : 1;
}

template<class T>
void TDefDestructor(void* d)
{
	delete static_cast<T*>(d);
}

template<class T>
void* TDefFactory(const void* k)
{
	return new T(*static_cast<const T*>(k));
}

template <class Key, class Data,
        VS_Map::Predicate		KeyPredicate	=	::TDefPredicate<Key>,
        VS_Map::Factory			KeyFactory		=	::TDefFactory<Key>,
        VS_Map::Factory			DataFactory		=	::TDefFactory<Data>,
        VS_Map::Destructor  KeyDestructor =	::TDefDestructor<Key>,
        VS_Map::Destructor	DataDestructor=	::TDefDestructor<Data> >
 class VST_Map: private VS_Map
 {
 public:
	class Exception
	{
	public:
		enum Code {UNKNOWN,NOT_FOUND};
		Code m_code;
		const VST_Map* m_map;
		Exception(const VST_Map* map, Code c=UNKNOWN) :m_code(c),m_map(map) {};
	};

	struct Pair
	{
		Pair () : key ( 0 ), data ( 0 ) {}
		Pair ( Key* key, Data* data ) : key ( key ), data ( data ) {}
		Key* key;
		Data* data;
	};
	typedef const Pair& ConstRef;
	typedef Pair& Reference;
	typedef const Pair* ConstPointer;
	typedef Pair* Pointer;

	class Iterator;
	class ConstIterator: public VS_Map::ConstIterator
	{
	public:
		ConstIterator () {}
		ConstIterator ( const Node* node, const VS_Map* map )
			:	VS_Map::ConstIterator(node, map ) {}
		ConstIterator ( const Iterator& it )
			: VS_Map::ConstIterator ( it.node,  it.container ) {}
		ConstRef operator* () const
		{	return (ConstRef)node->pair; }
		ConstPointer operator-> () const
		{	return (ConstPointer)&node->pair; }

		const VST_Map* cnt() const
		{ return (const VST_Map*)container; };

		const Data& data() const
		{
			if (operator!())
				throw Exception(cnt(), Exception::NOT_FOUND);
			return *static_cast<Data*>(node->pair.data);
		}
		const Key& key() const
		{
			if (operator!())
				throw Exception(cnt(), Exception::NOT_FOUND);
			return *static_cast<Key*>(node->pair.key);
		}

		bool operator!() const
		{	return node==cnt()->Tree().Nil(); }
		operator Data() const
		{
			if (operator!())
				throw Exception(cnt(), Exception::NOT_FOUND);
			return *static_cast<Data*>(node->pair.data);
		}
		operator const Data*() const
		{
			if (operator!())
				throw Exception(cnt(), Exception::NOT_FOUND);
			return static_cast<Data*>(node->pair.data);
		}


		friend class VST_Map;
	};
	class Iterator: public VS_Map::Iterator
	{
		const Key* new_key;
	public:
		Iterator () : new_key(0) {}
		virtual ~Iterator()
		{}
		Iterator ( Node* node, VST_Map* map )
			:	VS_Map::Iterator(node, map ), new_key(0)
		{}
		Iterator ( Node* node, VST_Map* map, const Key& key )
			:	VS_Map::Iterator(node, map ), new_key(&key)
		{}

		Reference operator*()
		{	return (Reference)node->pair; }
		Pointer operator->()
		{	return (Pointer)&node->pair; }
		VST_Map* cnt()
		{ return (VST_Map*)container; };
		const VST_Map* cnt() const
		{ return (const VST_Map*)container; };

		bool operator!() const
		{	return node==cnt()->Tree().Nil(); }

		Data& data()
		{
			if (operator!())
				throw Exception(cnt(), Exception::NOT_FOUND);
			return *static_cast<Data*>(node->pair.data);
		}
		const Key& key() const
		{
			if (operator!())
				throw Exception(cnt(), Exception::NOT_FOUND);
			return *static_cast<Key*>(node->pair.key);
		}

		operator Data() const
		{
			if (operator!())
				throw Exception(cnt(), Exception::NOT_FOUND);
			return *static_cast<Data*>(node->pair.data);
		}

		operator Data*()
		{
			if (operator!())
				throw Exception(cnt(), Exception::NOT_FOUND);
			return static_cast<Data*>(node->pair.data);
		}

		Iterator& operator=(const Data& data)
		{
			Node* nil_node=cnt()->Tree().Nil();
			if (node==nil_node && new_key==0)
				throw Exception(cnt(),Exception::NOT_FOUND);

			if (node==nil_node)
			{
				*this=cnt()->Assign(*new_key,data);
			}
			else
				cnt()->Assign(*this,data);
			return *this;
		}
		Iterator& operator=(const Data* data)
		{	return operator =(*data);	}

		bool operator== ( const Iterator& it ) const
		{	return node == it.node; }
		bool operator!= ( const Iterator& it ) const
		{	return node != it.node; }

		friend class VST_Map;
		friend class ConstIterator;
	};
	friend class Iterator;
	friend class ConstIterator;

// constructors
 	VST_Map()
	 {
		 SetPredicate(KeyPredicate);
		 SetKeyFactory(KeyFactory,KeyDestructor);
		 SetDataFactory(DataFactory,DataDestructor);
	 }
	VST_Map ( const VST_Map& m ) : VS_Map ( m )
	{	};

	~VST_Map () {};
	VST_Map& operator= ( const VST_Map& x )
	{
		this->VS_Map::operator=(x);
		return *this;
	}

	// Capacity
	/// Returns the length of the controlled sequence
	size_t Size () const
	{	return VS_Map::Size();	}
	/// returns true for an empty controlled sequence
	bool Empty () const
	{	return VS_Map::Empty();	}

	/// Find/Insert value with key. Returns iterator to this value.
	Iterator Assign ( const Key& key, const Data& data )
	{
		Node* i = tree.Assign ( VS_Map::Pair ( (void*)&key, (void*)&data ));
		return Iterator ( i, this );
	}
	Iterator Assign ( const Key* key, const Data& data )
	{	return Assign(*key, data);	};
	Iterator Assign ( const Key& key, const Data* data )
	{	return Assign( key, *data);	};
	Iterator Assign ( const Key* key, const Data* data )
	{	return Assign(*key, *data);	};
	/// Assign data to value of iterator it
	void Assign ( Iterator& it, const Data& data )
	{	tree.Assign ( it.node, &data );	}
	void Assign ( Iterator& it, const Data* data )
	{	tree.Assign ( it.node, data );	}

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


	/// pair(it, false).
	bool Insert ( const Pair& value )
	{	return VS_Map::Insert((const VS_Map::Pair&)value); };

	bool Insert ( const Key& key, const Data& data )
	{	return VS_Map::Insert(&key, &data); };
	bool Insert ( const Key& key, const Data* data )
	{	return VS_Map::Insert(&key, data); };
	bool Insert ( const Key* key, const Data& data )
	{	return VS_Map::Insert(key, &data); };
	bool Insert ( const Key* key, const Data* data )
	{	return VS_Map::Insert(key, data); };

	/// Also returns iterator to new node if successfully
	bool Insert ( Iterator& it, const Pair& value )
	{	return VS_Map::Insert(it,(const VS_Map::Pair&)value); };
	bool Insert ( Iterator& it, const Key& key, const Data& data )
	{	return VS_Map::Insert(it, &key, &data); };
	bool Insert ( Iterator& it, const Key& key, const Data* data )
	{	return VS_Map::Insert(it, &key, data); };
	bool Insert ( Iterator& it, const Key* key, const Data& data )
	{	return VS_Map::Insert(it, key, &data); };
	bool Insert ( Iterator& it, const Key* key, const Data* data )
	{	return VS_Map::Insert(it, key, data); };


	/// calls Erase( begin(), end()).
	void Clear ()
	{	VS_Map::Clear(); };

	/// returns an iterator that designates the first element remaining
	/// beyond any elements removed, or end() if no such element exists.
	/// removes the element of the controlled sequence pointed to by it
	Iterator Erase ( Iterator it )
	{
		Node* n = tree.Delete ( it.node );
		return Iterator ( n, this );
	}

	/// removes the elements in the interval [first, last)
	Iterator Erase ( Iterator first, Iterator last )
	{
		Node* i;
		for (i = first.node; i != last.node; )
			i = tree.Delete ( i );
		return Iterator ( i, this );
	}

	/// removes the elements with sort keys in the range
	/// [lower_bound(key), upper_bound(key)). It returns the number of
	/// elements it removes
	size_t Erase ( const Key& key )
		{	return VS_Map::Erase(&key); }
	size_t Erase ( const Key* key )
		{	return VS_Map::Erase(key); }
// swaps the controlled sequences between *this and x

	Iterator Find ( const Key& key )
	{
		Node* i = tree.Find ( &key );
		return Iterator ( i, this );
	}
	Iterator Find ( const Key* key )
	{return Find(*key);	};

	ConstIterator Find ( const Key& key ) const
	{
		Node* i = tree.Find ( &key );
		return ConstIterator ( i, this );
	}
	ConstIterator Find ( const Key* key ) const
	{	return Find(*key); }

	ConstIterator operator []( const Key& key ) const
	{	return Find(key);	}
	ConstIterator operator []( const Key* key ) const
	{	return Find(key);	}

	Iterator operator []( const Key& key )
	{
		Node* i = tree.Find ( &key );
		return Iterator ( i, this, key );
	}
	Iterator operator []( const Key* key )
	{	return operator[](*key);	}

protected:

	VS_RBTree& Tree()
	{	return tree; }
	const VS_RBTree& Tree() const
	{	return tree; }


 };

template <class Data,
          VS_Map::Factory			DataFactory		=	::TDefFactory<Data>,
          VS_Map::Destructor	DataDestructor=	::TDefDestructor<Data> >
class VST_StrMap:
  public VST_Map<char,Data,VS_Map::StrPredicate,
                VS_Map::StrFactory,DataFactory,
                VS_Map::StrDestructor,DataDestructor>
{
};

template <class Data,
          VS_Map::Factory			DataFactory		=	::TDefFactory<Data>,
          VS_Map::Destructor	DataDestructor=	::TDefDestructor<Data> >
class VST_StrIMap:
  public VST_Map<char,Data,VS_Map::StrIPredicate,
                VS_Map::StrFactory,DataFactory,
                VS_Map::StrDestructor,DataDestructor>
{
};

template <class Key,
          VS_Map::Predicate		KeyPredicate	=	::TDefPredicate<Key>,
          VS_Map::Factory			KeyFactory		=	::TDefFactory<Key>,
          VS_Map::Destructor	KeyDestructor=	::TDefDestructor<Key> >
class VST_StrDataMap:
  public VST_Map<Key,char,KeyPredicate,
                KeyFactory,VS_Map::StrFactory,
                KeyDestructor,VS_Map::StrDestructor>
{
};

typedef VST_Map<char,char,VS_Map::StrPredicate,
                VS_Map::StrFactory,VS_Map::StrFactory,
                VS_Map::StrDestructor,VS_Map::StrDestructor>
        VS_StrStrMap;

typedef VST_Map<char,char,VS_Map::StrIPredicate,
                VS_Map::StrFactory,VS_Map::StrFactory,
                VS_Map::StrDestructor,VS_Map::StrDestructor>
        VS_StrIStrMap;

typedef VST_StrIMap< int, VS_RBTree::VoidFactory, VS_RBTree::VoidDestructor >
		VS_StrINullMap;

typedef VST_StrIMap< int, VS_RBTree::IntFactory, VS_RBTree::IntDestructor >
		VS_StrI_IntMap;

 #endif //VS_MAP_TEMPLATE
