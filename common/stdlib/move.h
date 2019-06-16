#pragma once

#include <common/base.h>

namespace Stdlib
{

template< class T > struct RemoveReference      {typedef T type;};
template< class T > struct RemoveReference<T&>  {typedef T type;};
template< class T > struct RemoveReference<T&&> {typedef T type;};

template <typename T>
typename RemoveReference<T>::type&& Move(T&& arg)
{
    return static_cast<typename RemoveReference<T>::type&&>(arg);
}

template <class T>
T&& Forward(typename RemoveReference<T>::type& arg)
{
    return static_cast<T&&>(arg);
}

template <class T>
T&& Forward(typename RemoveReference<T>::type&& arg)
{
    return static_cast<T&&>(arg);
}

template <class T> void Swap(T& a, T& b)
{
    T c(Move(a)); a=Move(b); b=Move(c);
}

}