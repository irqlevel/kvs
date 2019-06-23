#pragma once

#include <common/base.h>
#include <common/sync/atomic.h>
#include <common/malloc/malloc.h>

#include "base.h"
#include "trace.h"
#include "deleter.h"

namespace Stdlib
{

template<class T, class Deleter = DefaultObjectDeleter<T>>
class ObjectReference final
{
private:
    static const int SharedPtrLL = 151;

public:
    ObjectReference(T* object)
        : object_(nullptr)
    {
        counter_.Set(1);
        object_ = object;

        Trace(SharedPtrLL, "objref 0x%p obj 0x%p ctor\n", this, object_);
    }

    ~ObjectReference()
    {
        Trace(SharedPtrLL, "objref 0x%p dtor\n", this);

        BUG_ON(object_ != nullptr);
        BUG_ON(counter_.Get() != 0);
    }

    void IncCounter()
    {
        counter_.Inc();
        Trace(SharedPtrLL, "objref 0x%p obj 0x%p inc counter %d\n", this, object_, counter_.Get());
    }

    int GetCounter()
    {
        return counter_.Get();
    }

    void SetObject(T *object)
    {
        if (BUG_ON(object_ != nullptr))
            return;

        object_ = object;
    }

    T* GetObject()
    {
        return object_;
    }

    bool DecCounter()
    {
        if (counter_.Dec() == 0)
        {
            Trace(SharedPtrLL, "objref 0x%p obj 0x%p dec counter %d\n", this, object_, counter_.Get());

            Deleter deleter;

            deleter(object_);

            object_ = nullptr;
            return true;
        }
        Trace(SharedPtrLL, "objref 0x%p obj 0x%p dec counter %d\n", this, object_, counter_.Get());

        return false;
    }

private:
    Sync::Atomic counter_;
    T* object_;

    ObjectReference() = delete;
    ObjectReference(const ObjectReference& other) = delete;
    ObjectReference(ObjectReference&& other) = delete;
    ObjectReference& operator=(const ObjectReference& other) = delete;
    ObjectReference& operator=(ObjectReference&& other) = delete;
};

template<class T, class Deleter = DefaultObjectDeleter<T>>
class SharedPtr final
{
private:

static const int SharedPtrLL = 150;

public:
    SharedPtr()
    {
        object_ref_ = nullptr;

        Trace(SharedPtrLL, "ptr 0x%p ctor obj 0x%p\n", this, Get());
    }

    SharedPtr(ObjectReference<T, Deleter> *object_ref)
    {
        object_ref_ = object_ref;

        Trace(SharedPtrLL, "ptr 0x%p ctor obj 0x%p\n", this, Get());
    }

    SharedPtr(T *object)
        : SharedPtr()
    {
        Reset(object);

        Trace(SharedPtrLL, "ptr 0x%p ctor obj 0x%p\n", this, Get());
    }

    SharedPtr(const SharedPtr<T>& other)
        : SharedPtr()
    {
        object_ref_ = other.object_ref_;
        if (object_ref_ != nullptr)
        {
            object_ref_->IncCounter();
        }

        Trace(SharedPtrLL, "ptr 0x%p ctor obj 0x%p\n", this, Get());
    }

    SharedPtr(SharedPtr<T>&& other)
        : SharedPtr()
    {
        object_ref_ = other.object_ref_;
        other.object_ref_ = nullptr;

        Trace(SharedPtrLL, "ptr 0x%p ctor obj 0x%p\n", this, Get());
    }

    SharedPtr<T>& operator=(const SharedPtr<T>& other)
    {
        if (this != &other)
        {
            Reset(nullptr);
            object_ref_ = other.object_ref_;
            if (object_ref_ != nullptr)
            {
                object_ref_->IncCounter();
            }
        }

        Trace(SharedPtrLL, "ptr 0x%p op= obj 0x%p\n", this, Get());

        return *this;
    }

    SharedPtr<T>& operator=(SharedPtr<T>&& other)
    {
        if (this != &other)
        {
            Reset(nullptr);
            object_ref_ = other.object_ref_;
            other.object_ref_ = nullptr;
        }

        Trace(SharedPtrLL, "ptr 0x%p op= obj 0x%p\n", this, Get());

        return *this;
    }

    T* Get() const
    {
        return (object_ref_ != nullptr) ? object_ref_->GetObject() : nullptr;
    }

    T& operator*() const
    {
        return *Get();
    }

    T* operator->() const
    {
        return Get();
    }

    int GetCounter()
    {
        return (object_ref_ != nullptr) ? object_ref_->GetCounter() : 0;
    }

    ~SharedPtr()
    {
        Reset(nullptr);
    }

    void Reset(T* object)
    {
        Trace(SharedPtrLL, "ptr 0x%p reset obj 0x%p new 0x%p\n", this, Get(), object);

        BUG_ON(Get() != nullptr && Get() == object);

        if (object_ref_ != nullptr)
        {
            if (object_ref_->DecCounter())
            {
                delete object_ref_;
            }
        }

        object_ref_ = nullptr;

        if (object != nullptr)
        {
            object_ref_ = TAlloc<ObjectReference<T>>(object);
            if (object_ref_ == nullptr)
            {
                return;
            }
        }
    }

    void Reset()
    {
        Reset(nullptr);
    }

private:
    ObjectReference<T, Deleter>* object_ref_;
};

template<class T, class Deleter = DefaultObjectDeleter<T>, class... Args>
SharedPtr<T, Deleter> MakeShared(Args&&... args)
{
    ObjectReference<T, Deleter>* object_ref = TAlloc<ObjectReference<T, Deleter>>(nullptr);
    if (object_ref == nullptr)
        return SharedPtr<T, Deleter>();

    T* object = TAlloc<T>(Stdlib::Forward<Args>(args)...);
    if (object == nullptr)
    {
        delete object_ref;
        return SharedPtr<T, Deleter>();
    }

    object_ref->SetObject(object);
    return SharedPtr<T, Deleter>(object_ref);
}

}