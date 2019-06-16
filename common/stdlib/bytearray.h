#pragma once

#include "base.h"

namespace Stdlib
{

template<class T>
class ByteArray
{
public:
    ByteArray()
        : Arr(nullptr), Size(0), Capacity(0)
    {
    }

    size_t GetSize() const
    {
        return Size;
    }

    size_t GetCapacity() const
    {
        return Capacity;
    }

    T& operator[](size_t index)
    {
        BUG_ON(index < 0 || index >= Size);
        return Arr[index];
    }

    bool Reserve(size_t capacity)
    {
        if (capacity <= Capacity)
            return true;

        T* newArr = new T[capacity];
        if (!newArr)
            return false;

        if (Arr)
        {
            Stdlib::MemCpy(newArr, Arr, Size * sizeof(T));
            delete[] Arr;
        }
        Arr = newArr;
        Capacity = capacity;
        return true;
    }

    bool ReserveAndUse(size_t capacity)
    {
        if (!Reserve(capacity))
            return false;
        Size = Capacity;
        return true;
    }

    bool CopyFrom(const ByteArray<T>& other) {
        if (!ReserveAndUse(other.GetSize()))
            return false;

        Stdlib::MemCpy(Arr, other.Arr, other.GetSize() * sizeof(T));
        return true;
    }

    bool Append(const ByteArray<T>& other) {
        return Append(other.GetConstBuf(), other.GetSize());
    }

    bool Append(const T* buf, size_t bufSize) {
        size_t size = Size;

        if (!ReserveAndUse(size + bufSize))
            return false;

        Stdlib::MemCpy(&Arr[size], buf, bufSize * sizeof(T));
        return true;
    }

    bool Truncate(size_t size)
    {
        if (size > Size)
            return false;

        Size = size;
        return true;
    }

    bool PushBack(T&& e)
    {
        if (Size == Capacity)
        {
            if (!Reserve(2*Size + 1))
                return false;
        }
        Arr[Size++] = Stdlib::Move(e);
        return true;
    }

    bool PushBack(const T& e)
    {
        if (Size == Capacity)
        {
            if (!Reserve(2*Size + 1))
                return false;
        }
        Arr[Size++] = e;
        return true;
    }

    const T* GetConstBuf() const
    {
        return Arr;
    }

    T* GetBuf()
    {
        return Arr;
    }

    virtual ~ByteArray()
    {
        Release();
    }

    void Clear()
    {
        Release();
    }

    ByteArray(ByteArray&& other)
        : ByteArray()
    {
        Arr = other.Arr;
        Size = other.Size;
        Capacity = other.Capacity;
        other.Arr = nullptr;
        other.Size = 0;
        other.Capacity = 0;
    }

    ByteArray& operator=(ByteArray&& other)
    {
        Release();
        Arr = other.Arr;
        Size = other.Size;
        Capacity = other.Capacity;
        other.Arr = nullptr;
        other.Size = 0;
        other.Capacity = 0;
        return *this;
    }

    bool operator==(const ByteArray& other) {
        if (Size != other.Size)
            return false;
        
        if (Stdlib::MemCmp(Arr, other.Arr, Size * sizeof(T)))
            return false;

        return true;
    }

    ByteArray(const ByteArray& other)
        : ByteArray()
    {
        Arr = new T[other.Capacity];
        if (Arr == nullptr)
        {
            return;
        }

        Stdlib::MemCpy(Arr, other.Arr, other.Size * sizeof(T));
        Size = other.Size;
        Capacity = other.Capacity;
    }

    ByteArray& operator=(const ByteArray& other)
    {
        if (this != &other) {
            Release();

            Arr = new T[other.Capacity];
            if (Arr == nullptr)
            {
                return *this;
            }

            Stdlib::MemCpy(Arr, other.Arr, other.Size * sizeof(T));
            Size = other.Size;
            Capacity = other.Capacity;
        }
        return *this;
    }

    void Shuffle()
    {
        BUG_ON(1);
    }

    ByteArray(const T* arr, size_t size)
        : ByteArray()
    {
        Arr = new T[size];
        if (Arr == nullptr)
        {
            return;
        }
        
        Stdlib::MemCpy(Arr, arr, size * sizeof(T));
        Size = size;
        Capacity = Size;
    }

private:
    void Release()
    {
        if (Arr)
        {
            delete[] Arr;
            Arr = nullptr;
        }
        Size = 0;
        Capacity = 0;
    }

    T* Arr;
    size_t Size;
    size_t Capacity;
};

}
