#pragma once

#include <common/base.h>
#include <common/sync/atomic.h>
#include <common/malloc/malloc.h>

#include "base.h"
#include "trace.h"

namespace Stdlib
{

template<typename T>
class ObjectReference final
{
private:
    static const int SharedPtrLL = 151;

public:
    ObjectReference(T* object)
        : Object(nullptr)
    {
        Counter.Set(1);
        Object = object;

        Trace(SharedPtrLL, "objref 0x%p obj 0x%p ctor\n", this, Object);
    }

    ~ObjectReference()
    {
        Trace(SharedPtrLL, "objref 0x%p dtor\n", this);

        BUG_ON(Object != nullptr);
        BUG_ON(Counter.Get() != 0);
    }

    void IncCounter()
    {
        Counter.Inc();
        Trace(SharedPtrLL, "objref 0x%p obj 0x%p inc counter %d\n", this, Object, Counter.Get());
    }

    int GetCounter()
    {
        return Counter.Get();
    }

    void SetObject(T *object)
    {
        if (BUG_ON(Object != nullptr))
            return;

        Object = object;
    }

    T* GetObject()
    {
        return Object;
    }

    bool DecCounter()
    {
        if (Counter.Dec() == 0)
        {
            Trace(SharedPtrLL, "objref 0x%p obj 0x%p dec counter %d\n", this, Object, Counter.Get());

            delete Object;
            Object = nullptr;
            return true;
        }
        Trace(SharedPtrLL, "objref 0x%p obj 0x%p dec counter %d\n", this, Object, Counter.Get());

        return false;
    }

private:
    Sync::Atomic Counter;
    T* Object;

    ObjectReference() = delete;
    ObjectReference(const ObjectReference& other) = delete;
    ObjectReference(ObjectReference&& other) = delete;
    ObjectReference& operator=(const ObjectReference& other) = delete;
    ObjectReference& operator=(ObjectReference&& other) = delete;
};

template<typename T>
class SharedPtr final
{
private:

static const int SharedPtrLL = 150;

public:
    SharedPtr()
    {
        ObjectRef = nullptr;

        Trace(SharedPtrLL, "ptr 0x%p ctor obj 0x%p\n", this, Get());
    }

    SharedPtr(ObjectReference<T> *objectRef)
    {
        ObjectRef = objectRef;

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
        ObjectRef = other.ObjectRef;
        if (ObjectRef != nullptr)
        {
            ObjectRef->IncCounter();
        }

        Trace(SharedPtrLL, "ptr 0x%p ctor obj 0x%p\n", this, Get());
    }

    SharedPtr(SharedPtr<T>&& other)
        : SharedPtr()
    {
        ObjectRef = other.ObjectRef;
        other.ObjectRef = nullptr;

        Trace(SharedPtrLL, "ptr 0x%p ctor obj 0x%p\n", this, Get());
    }

    SharedPtr<T>& operator=(const SharedPtr<T>& other)
    {
        if (this != &other)
        {
            Reset(nullptr);
            ObjectRef = other.ObjectRef;
            if (ObjectRef != nullptr)
            {
                ObjectRef->IncCounter();
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
            ObjectRef = other.ObjectRef;
            other.ObjectRef = nullptr;
        }

        Trace(SharedPtrLL, "ptr 0x%p op= obj 0x%p\n", this, Get());

        return *this;
    }

    T* Get() const
    {
        return (ObjectRef != nullptr) ? ObjectRef->GetObject() : nullptr;
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
        return (ObjectRef != nullptr) ? ObjectRef->GetCounter() : 0;
    }

    ~SharedPtr()
    {
        Reset(nullptr);
    }

    void Reset(T* object)
    {
        Trace(SharedPtrLL, "ptr 0x%p reset obj 0x%p new 0x%p\n", this, Get(), object);

        BUG_ON(Get() != nullptr && Get() == object);

        if (ObjectRef != nullptr)
        {
            if (ObjectRef->DecCounter())
            {
                delete ObjectRef;
            }
        }

        ObjectRef = nullptr;

        if (object != nullptr)
        {
            ObjectRef = TAlloc<ObjectReference<T>>(object);
            if (ObjectRef == nullptr)
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
    ObjectReference<T>* ObjectRef;
};

template<typename T, class... Args>
SharedPtr<T> MakeShared(Args&&... args)
{
    ObjectReference<T>* objRef = TAlloc<ObjectReference<T>>(nullptr);
    if (objRef == nullptr)
        return SharedPtr<T>();

    T* object = TAlloc<T>(Stdlib::Forward<Args>(args)...);
    if (object == nullptr)
    {
        delete objRef;
        return SharedPtr<T>();
    }

    objRef->SetObject(object);
    return SharedPtr<T>(objRef);
}

}