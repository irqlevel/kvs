#pragma once

namespace Stdlib
{

template<typename T1, typename T2>
class Pair
{
public:
    Pair()
        : first_()
        , second_()
    {
    }

    Pair(const T1& first, const T2& second)
        : first_(first)
        , second_(second)
    {
    }

    virtual ~Pair()
    {
    }

    Pair(const Pair& other)
    {
        first_ = other.first_;
        second_ = other.second_;
    }

    Pair& operator=(const Pair& other)
    {
        if (this != &other)
        {
            first_ = other.first_;
            second_ = other.second_;
        }
        return *this;
    }

    Pair(Pair&& other)
    {
        first_ = Move(other.first_);
        second_ = Move(other.second_);
    }

    Pair& operator=(Pair&& other)
    {
        if (this != &other)
        {
            first_ = Move(other.first_);
            second_ = Move(other.second_);
        }
        return *this;
    }

    T1& GetFirst() {
        return first_;
    }

    T2& GetSecond() {
        return second_;
    }

private:
    T1 first_;
    T2 second_;
};

}