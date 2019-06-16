#pragma once

#include "move.h"

namespace Stdlib
{

template<typename TValue, typename TError>
class Result
{
public:
    Result()
        : value_()
        , error_()
    {
    }

    Result(const TError& error)
        : value_()
        , error_(error)
    {
    }

    Result(TValue&& value, TError&& error)
        : value_(Move(value))
        , error_(Move(error))
    {
    }

    Result(const TValue& value, const TError& error)
        : value_(value)
        , error_(error)
    {
    }

    virtual ~Result()
    {
    }

    Result(const Result& other)
    {
        error_ = other.error_;
        value_ = other.value_;
    }

    Result& operator=(const Result& other)
    {
        if (this != &other)
        {
            error_ = other.error_;
            value_ = other.value_;
        }
        return *this;
    }

    Result(Result&& other)
    {
        error_ = Move(other.error_);
        value_ = Move(other.value_);
    }

    Result& operator=(Result&& other)
    {
        if (this != &other)
        {
            error_ = Move(other.error_);
            value_ = Move(other.value_);
        }
        return *this;
    }

    const TValue& Value() const {
        return value_;
    }

    const TError& Error() const {
        return error_;
    }

    TValue& MutValue() {
        return value_;
    }

    TError& MutError() {
        return error_;
    }

    void SetError(const TError& error) {
        error_ = error;
    }

    void SetValue(const TValue& value) {
        value_ = value;
    }

    void SetError(TError&& error) {
        error_ = Move(error);
    }

    void SetValue(TValue&& value) {
        value_ = Move(value);
    }

private:
    TValue value_;
    TError error_;

};

}