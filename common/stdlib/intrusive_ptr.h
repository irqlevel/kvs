#pragma once

#include <common/base.h>
#include <common/sync/atomic.h>
#include <common/malloc/malloc.h>

#include "base.h"
#include "trace.h"
#include "deleter.h"

namespace Stdlib
{

template<class T, class ObjectIncRef, class ObjectDecRef>
class IntrusivePtr final
{
private:

static const int IntrusivePtrLL = 150;

public:
    IntrusivePtr()
    {
        object_ = nullptr;

        Trace(IntrusivePtrLL, "ptr 0x%p ctor obj 0x%p\n", this, Get());
    }

    IntrusivePtr(T *object)
        : object_(object)
    {
        if (object_ != nullptr) {
            ObjectIncRef inc_ref;

            inc_ref(object_);
        }

        Trace(IntrusivePtrLL, "ptr 0x%p ctor obj 0x%p\n", this, Get());
    }

    IntrusivePtr(const IntrusivePtr& other)
        : IntrusivePtr()
    {
        if (other.object_ != nullptr) {
            ObjectIncRef inc_ref;

            inc_ref(other.object_);
            object_ = other.object_;
        }

        Trace(IntrusivePtrLL, "ptr 0x%p ctor obj 0x%p\n", this, Get());
    }

    IntrusivePtr(IntrusivePtr&& other)
        : IntrusivePtr()
    {
        object_ = other.object_;
        other.object_ = nullptr;

        Trace(IntrusivePtrLL, "ptr 0x%p ctor obj 0x%p\n", this, Get());
    }

    IntrusivePtr& operator=(const IntrusivePtr& other)
    {
        if (this != &other) {
            Reset(nullptr);

            if (other.object_ != nullptr) {
                ObjectIncRef  inc_ref;

                inc_ref(other.object_);
                object_ = other.object_;
            }
        }

        Trace(IntrusivePtrLL, "ptr 0x%p op= obj 0x%p\n", this, Get());

        return *this;
    }

    IntrusivePtr& operator=(IntrusivePtr&& other)
    {
        if (this != &other) {
            Reset(nullptr);

            object_ = other.object_;
            other.object_ = nullptr;
        }

        Trace(IntrusivePtrLL, "ptr 0x%p op= obj 0x%p\n", this, Get());

        return *this;
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


    ~IntrusivePtr()
    {
        Reset(nullptr);
    }

    void Reset(T* object)
    {
        Trace(IntrusivePtrLL, "ptr 0x%p reset obj 0x%p new 0x%p\n", this, Get(), object);

        BUG_ON(Get() != nullptr && Get() == object);

        if (object_ != nullptr) {
            ObjectDecRef dec_ref;
            
            dec_ref(object_);
            object_ = nullptr;
        }

        if (object != nullptr) {
            ObjectIncRef inc_ref;

            inc_ref(object);
            object_ = object;
        }
    }

    void Reset()
    {
        Reset(nullptr);
    }

    bool operator==(const IntrusivePtr& other) const {
        if (object_ == other.object_)
            return true;

        return false;
    }

private:
    T *object_;
};

}