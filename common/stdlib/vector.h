#pragma once

#include "base.h"

namespace Stdlib
{

template<class T>
class Vector
{
public:
    Vector()
        : arr_(nullptr), size_(0), capacity_(0)
    {
    }

    size_t GetSize() const
    {
        return size_;
    }

    size_t GetCapacity() const
    {
        return capacity_;
    }

    T& operator[](size_t index)
    {
        BUG_ON(index < 0 || index >= size_);
        return arr_[index];
    }

    bool Reserve(size_t capacity)
    {
        if (capacity <= capacity_)
            return true;

        T* new_arr = new T[capacity];
        if (!new_arr)
            return false;

        if (arr_)
        {
            for (size_t i = 0; i < size_; i++)
            {
                new_arr[i] = Stdlib::Move(arr_[i]);
            }
            delete[] arr_;
        }
        arr_ = new_arr;
        capacity_ = capacity;
        return true;
    }

    bool ReserveAndUse(size_t capacity)
    {
        if (!Reserve(capacity))
            return false;
        size_ = capacity;
        return true;
    }

    bool CopyFrom(const Vector<T>& other) {
        if (!ReserveAndUse(other.GetSize()))
            return false;

        for (size_t index = 0; index < other.GetSize(); index++)
            arr_[index] = other.GetConstBuf()[index];

        return true;
    }

    bool Append(const Vector<T>& other) {
        return Append(other.GetConstBuf(), other.GetSize());
    }

    bool Append(const T* buf, size_t buf_size) {
        size_t size = size_;

        if (!ReserveAndUse(size + buf_size))
            return false;

        for (size_t index = 0; index < buf_size; index++)
            arr_[size + index] = buf[index];

        return true;
    }

    bool Truncate(size_t size)
    {
        if (size > size_)
            return false;

        size_ = size;
        return true;
    }

    bool PushBack(T&& e)
    {
        if (size_ == capacity_)
        {
            if (!Reserve(2*size_ + 1))
                return false;
        }
        arr_[size_++] = Stdlib::Move(e);
        return true;
    }

    bool PushBack(const T& e)
    {
        if (size_ == capacity_)
        {
            if (!Reserve(2*size_ + 1))
                return false;
        }
        arr_[size_++] = e;
        return true;
    }

    const T* GetConstBuf() const
    {
        return arr_;
    }

    T* GetBuf()
    {
        return arr_;
    }

    virtual ~Vector()
    {
        Release();
    }

    void Clear()
    {
        Release();
    }

    Vector(Vector&& other)
        : Vector()
    {
        arr_ = other.arr_;
        size_ = other.size_;
        capacity_ = other.capacity_;
        other.arr_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    Vector& operator=(Vector&& other)
    {
        Release();
        arr_ = other.arr_;
        size_ = other.size_;
        capacity_ = other.capacity_;
        other.arr_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
        return *this;
    }

    bool operator==(const Vector& other) {
        if (size_ != other.size_)
            return false;
        
        for (size_t i = 0; i < size_; i++)
            if (arr_[i] != other.arr_[i])
                return false;

        return true;
    }

    Vector(const Vector& other)
        : Vector()
    {
        arr_ = new T[other.capacity_];
        if (arr_ == nullptr)
        {
            return;
        }

        for (size_t i = 0; i < other.size_; i++)
        {
            arr_[i] = other.arr_[i];
        }
        size_ = other.size_;
        capacity_ = other.capacity_;
    }

    Vector& operator=(const Vector& other)
    {
        if (this != &other) {
            Release();

            arr_ = new T[other.capacity_];
            if (arr_ == nullptr)
            {
                return *this;
            }

            for (size_t i = 0; i < other.size_; i++)
            {
                arr_[i] = other.arr_[i];
            }
            size_ = other.size_;
            capacity_ = other.capacity_;
        }
        return *this;
    }

    void Shuffle()
    {
        BUG_ON(1);
    }

    Vector(const T* arr, size_t size)
        : Vector()
    {
        arr_ = new T[size];
        if (arr_ == nullptr)
        {
            return;
        }

        for (size_t i = 0; i < size; i++)
        {
            arr_[i] = arr[i];
        }

        size_ = size;
        capacity_ = size;
    }

private:
    void Release()
    {
        if (arr_ != nullptr)
        {
            delete[] arr_;
            arr_ = nullptr;
        }
        size_ = 0;
        capacity_ = 0;
    }

    T* arr_;
    size_t size_;
    size_t capacity_;
};

}
