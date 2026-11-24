//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Kürzlich verwendete <T>s                               					            //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <cassert>
#include <list>

//////////////////////////////////////////////////////////////////////////////////////////
template<class T>
class LRUCache
{
    // Daten /////////////////////////////////////////////////////////////////////////////
    private:
        int           mCapacity;
        std::list<T>  mList;

    // Funktionen ////////////////////////////////////////////////////////////////////////
    public:
            explicit  LRUCache(int);
            int       capacity(void) const;
            void      clear(void);
            bool      isEmpty(void) const;
			int		  count(void) const;
			const T&  at(int) const;
            void      use(const T&);
			const T&  recent(const T& = T()) const;
};

//////////////////////////////////////////////////////////////////////////////////////////
template<class T>
LRUCache<T>::LRUCache(int capacity)
{
    mCapacity = capacity;
}

//////////////////////////////////////////////////////////////////////////////////////////
template<class T> int
LRUCache<T>::capacity(void) const
{
    return mCapacity;
}

//////////////////////////////////////////////////////////////////////////////////////////
template<class T> void
LRUCache<T>::clear(void)
{
    mList.clear();
}

//////////////////////////////////////////////////////////////////////////////////////////
template<class T> bool
LRUCache<T>::isEmpty(void) const
{
    return mList.empty();
}

//////////////////////////////////////////////////////////////////////////////////////////
template<class T> int
LRUCache<T>::count(void) const
{
    return mList.size();
}

//////////////////////////////////////////////////////////////////////////////////////////
template<class T> const T&
LRUCache<T>::at(int position) const
{
    assert (position >= 0 && position < int(mList.size()));
    typename std::list<T>::const_iterator it = mList.cbegin();
    while (position > 0)
    {   position--;
        it++;
    }
    return *it;
}

//////////////////////////////////////////////////////////////////////////////////////////
template<class T> void
LRUCache<T>::use(const T& type)
{
    for (typename std::list<T>::iterator it = mList.begin(); it != mList.end(); it++)
    {   if (*it == type)
        {   mList.erase(it);
            break;
        }
    }
    mList.push_front(type);
    if (int(mList.size()) > mCapacity) mList.pop_back();
}

//////////////////////////////////////////////////////////////////////////////////////////
template<class T> const T&
LRUCache<T>::recent(const T& deflt) const
{
    if (mList.empty()) return deflt;
    return at(0);
}

//////////////////////////////////////////////////////////////////////////////////////////
