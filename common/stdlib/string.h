#pragma once

#include "base.h"
#include "bytearray.h"
#include "result.h"
#include "hash.h"
#include "error.h"
#include <string.h>

#include <errno.h>

namespace Stdlib
{
    class String
    {
    public:
        String()
            : Arr(nullptr)
            , Length(0)
            , Capacity(0)
        {
        }

        virtual ~String()
        {
            Release();
        }

        String(const char *s)
            : String()
        {
            size_t len = StrLen(s);
            if (!ReserveAndUse(len))
                return;
            
            MemCpy(GetBuf(), s, len);
            GetBuf()[len] = '\0';
        }

        String(const char *s, size_t len)
            : String()
        {
            if (!ReserveAndUse(len))
                return;
            
            MemCpy(GetBuf(), s, len);
            GetBuf()[len] = '\0';
        }

        bool Reset(const char *s, size_t len)
        {
            Clear();
            if (!ReserveAndUse(len))
                return false;

            MemCpy(GetBuf(), s, len);
            GetBuf()[len] = '\0';
            return true;
        }

        bool Reset(const char *s)
        {
            size_t len = strlen(s);
            return Reset(s, len);
        }

        bool Reset(const String& s)
        {
            return Reset(s.GetConstBuf(), s.GetLength());
        }

        bool Equal(const char *s) const
        {
            if (Stdlib::StrnCmp(GetConstBuf(), s, GetLength() + 1) == 0)
                return true;
            return false;
        }

        bool HasPrefix(const char *prefix) const
        {
            size_t len = Stdlib::StrLen(prefix);
            if (len > GetLength())
                return false;

            if (Stdlib::StrnCmp(GetConstBuf(), prefix, len) == 0)
                return true;
            return false;
        }

        int Rfind(char c) const {
            auto buf = GetConstBuf();
            for (int pos = GetLength() - 1; pos >= 0; pos--) {
                if (buf[pos] == c)
                    return pos;
            }

            return -1;
        }

        int Find(char c, size_t pos = 0) const {
            auto buf = GetConstBuf();
            for (pos = 0; pos < GetLength(); pos++)
                if (buf[pos] == c)
                    return pos;
            
            return -1;
        }

        //[start .. end)
        Result<String, Stdlib::Error> Substring(size_t start, size_t end) const {
            Result<String, Stdlib::Error> result;
            if (start >= end) {
                result.SetError(STDLIB_ERRNO_ERROR(EINVAL));
                return result;
            }

            if (start >= GetLength()) {
                result.SetError(STDLIB_ERRNO_ERROR(EINVAL));
                return result;
            }

            if (end > GetLength())
                end = GetLength();

            size_t len = end - start;
            if (!result.MutValue().ReserveAndUse(len)) {
                result.SetError(STDLIB_ERRNO_ERROR(ENOMEM));
                return result;
            }

            Stdlib::MemCpy(result.MutValue().GetBuf(), GetConstBuf() + start, len);
            result.MutValue().GetBuf()[len] = '\0';
            result.SetError(0);
            return result;
        }

        size_t Hash() const {
            return Stdlib::Djb2Hash(GetConstBuf(), GetLength());
        }

        static size_t Hash(const String& s) {
            return s.Hash();
        }

        const char *GetConstBuf() const {
            return Arr;
        }

        char *GetBuf() {
            return Arr;
        }

        size_t GetLength() const
        {
            return Length;
        }

        void Clear()
        {
            Release();
        }

        char operator[](size_t index)
        {
            BUG_ON(index < 0 || index >= (Length + 1));
            return Arr[index];
        }

        bool CopyFrom(const String& other) {
            if (!ReserveAndUse(other.GetLength()))
                return false;

            for (size_t index = 0; index < (other.GetLength() + 1); index++)
                Arr[index] = other.GetConstBuf()[index];

            return true;
        }

        bool Append(const String& other) {
            return Append(other.GetConstBuf(), other.GetLength());
        }

        bool Append(const char* buf, size_t bufLength) {
            size_t length = GetLength();

            if (!ReserveAndUse(length + bufLength))
                return false;

            for (size_t index = 0; index < bufLength; index++)
                Arr[length + index] = buf[index];

            Arr[length + bufLength] = '\0';
            return true;
        }

        bool Append(char c) {
            return Append(&c, 1);
        }

        String(String&& other)
            : String()
        {
            Arr = other.Arr;
            Length = other.Length;
            Capacity = other.Capacity;
            other.Arr = nullptr;
            other.Length = 0;
            other.Capacity = 0;
        }

        String& operator=(String&& other)
        {
            Release();
            Arr = other.Arr;
            Length = other.Length;
            Capacity = other.Capacity;
            other.Arr = nullptr;
            other.Length = 0;
            other.Capacity = 0;
            return *this;
        }

        bool operator==(const String& other) {
            if (Length != other.Length)
                return false;
            
            for (size_t i = 0; i < Length; i++)
                if (Arr[i] != other.Arr[i])
                    return false;

            return true;
        }

        bool operator!=(const String &other) {
            return !(this->operator==(other));
        }

        String(const String& other)
            : String()
        {
            Arr = new char[other.Length + 1];
            if (Arr == nullptr)
            {
                return;
            }

            for (size_t i = 0; i < (other.Length + 1); i++)
            {
                Arr[i] = other.Arr[i];
            }
            Length = other.Length;
            Capacity = other.Length + 1;
        }

        String& operator=(const String& other)
        {
            if (this != &other) {
                Release();

                Arr = new char[other.Length + 1];
                if (Arr == nullptr)
                {
                    return *this;
                }

                for (size_t i = 0; i < (other.Length + 1); i++)
                {
                    Arr[i] = other.Arr[i];
                }
                Length = other.Length;
                Capacity = other.Length + 1;
            }
            return *this;
        }

        static bool IsHexDigit(char c)
        {
            if (c >= '0' && c <= '9')
                return true;
            if (c >= 'a' && c <= 'f')
                return true;
            if (c >= 'A' && c <= 'F')
                return true;
            return false;
        }

        Result<ulong, Stdlib::Error> ToUlong(int base = 10)
        {
            return Str2Ulong(GetConstBuf(), base);
        }

        bool EncodeToHex(void *buf, size_t buf_size)
        {
            if (!ReserveAndUse(2 * buf_size))
                return false;

            for (size_t i = 0; i < buf_size; i++) {
                unsigned char c = *(static_cast<const unsigned char *>(buf) + i);
                SnPrintf(Arr + 2 * i, 3, "%02x", c);
            }

            return true;
        }

    private:
        bool Reserve(size_t capacity)
        {
            if (capacity <= Capacity)
                return true;

            char* newArr = new char[capacity];
            if (!newArr)
                return false;

            if (Arr)
            {
                for (size_t i = 0; i < (Length + 1); i++)
                {
                    newArr[i] = Arr[i];
                }
                delete[] Arr;
            }
            Arr = newArr;
            Capacity = capacity;
            return true;
        }

        bool ReserveAndUse(size_t length)
        {
            if (!Reserve(length + 1))
                return false;
            
            Arr[length] = '\0';
            Length = length;
            return true;
        }

        void Release()
        {
            if (Arr != nullptr) {
                delete[] Arr;
                Arr = nullptr;
            }
            Length = 0;
            Capacity = 0;
        }

        char *Arr;
        size_t Length;
        size_t Capacity;
    };
}