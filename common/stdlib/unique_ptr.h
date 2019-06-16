#pragma once

#include "base.h"

#include <common/malloc/malloc.h>

namespace Stdlib
{

template<typename T>
class UniquePtr final
{
public:
    UniquePtr(UniquePtr&& other)
        : UniquePtr()
    {
        Reset(other.Release());
    }

    UniquePtr& operator=(UniquePtr&& other)
    {
        if (this != &other)
        {
            Reset(other.Release());
        }
        return *this;
    }

    UniquePtr()
        : Object(nullptr)
    {
    }

    UniquePtr(T* object)
        : UniquePtr()
    {
        Reset(object);
    }

    UniquePtr(const UniquePtr& other) = delete;

    UniquePtr& operator=(const UniquePtr& other) = delete;

    void Reset(T* object)
    {
        BUG_ON(Object != nullptr && Object == object);

        if (Object != nullptr)
        {
            //TODO:panic(get_kapi()->unique_key_unregister(Object, this) != 0);
            delete Object;
            Object = nullptr;
        }

        Object = object;
        //TODO:if (Object != nullptr)
        //  panic(get_kapi()->unique_key_register(Object, this, get_kapi_pool_type(PoolType)) != 0);
    }

    void Reset()
    {
        Reset(nullptr);
    }

    T* Release()
    {
        T* object = Object;

        //TODO:if (object != nullptr)
        //    panic(get_kapi()->unique_key_unregister(object, this) != 0);

        Object = nullptr;
        return object;
    }

    ~UniquePtr()
    {
        Reset(nullptr);
    }

    T* Get() const
    {
        return Object;
    }

    T& operator*() const
    {
        return *Get();
    }

    T* operator->() const
    {
        return Get();
    }

private:
    T* Object;
};

template<typename T, class... Args>
UniquePtr<T> MakeUnique(Args&&... args)
{
    return UniquePtr<T>(TAlloc<T>(Stdlib::Forward<Args>(args)...));
}

}