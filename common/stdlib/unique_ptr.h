#pragma once

#include "base.h"
#include "deleter.h"

#include <common/malloc/malloc.h>

namespace Stdlib
{

template<class T, class Deleter = DefaultObjectDeleter<T>>
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
        : object_(nullptr)
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
        BUG_ON(object_ != nullptr && object_ == object);

        if (object_ != nullptr)
        {
            //TODO:panic(get_kapi()->unique_key_unregister(Object, this) != 0);
            delete object_;
            object_ = nullptr;
        }

        object_ = object;
        //TODO:if (Object != nullptr)
        //  panic(get_kapi()->unique_key_register(Object, this, get_kapi_pool_type(PoolType)) != 0);
    }

    void Reset()
    {
        Reset(nullptr);
    }

    T* Release()
    {
        T* object = object_;

        //TODO:if (object != nullptr)
        //    panic(get_kapi()->unique_key_unregister(object, this) != 0);

        object_ = nullptr;
        return object;
    }

    ~UniquePtr()
    {
        Reset(nullptr);
    }

    T* Get() const
    {
        return object_;
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
    T* object_;
    Deleter deleter_;
};

template<class T, class Deleter = DefaultObjectDeleter<T>, class... Args>
UniquePtr<T, Deleter> MakeUnique(Args&&... args)
{
    return UniquePtr<T, Deleter>(TAlloc<T>(Stdlib::Forward<Args>(args)...));
}

}