#pragma once

#include <common/base.h>
#include <common/malloc/malloc.h>
#include <common/stdlib/pair.h>
#include "list_entry.h"

namespace Stdlib
{

template<typename K, typename V, size_t NumBuckets, size_t (*Hash)(const K& key)>
class HashMap
{
public:
    class Iterator
    {
    public:
        void Next()
        {
            if (CurrListEntry != nullptr) {
                auto& bucket = MapRef.Bucket[BucketIndex];
                if (CurrListEntry->Flink != &bucket.NodeList)
                    CurrListEntry = CurrListEntry->Flink;
                else {
                    CurrListEntry = nullptr;
                    BucketIndex++;
                    NextBucket();
                }
            }
        }

        virtual ~Iterator() {}

        Iterator& operator++()
        {
            Next();
            return *this;
        }

        Iterator operator++(int)
        {
            Next();
            return *this;
        }

        V& operator*() const
        {
            auto node = CONTAINING_RECORD(CurrListEntry, Node, Link);
            return node->Value;
        }

        bool operator !=(const Iterator &other) const
        {
            return (CurrListEntry != other.CurrListEntry);
        }

        bool operator ==(const Iterator &other) const
        {
            return (CurrListEntry == other.CurrListEntry);
        }

        Iterator(HashMap& map, size_t bucketIndex, Stdlib::ListEntry* currListEntry)
            : MapRef(map)
            , BucketIndex(bucketIndex)
            , CurrListEntry(currListEntry)
        {
        }

        Iterator(HashMap& map)
            : Iterator(map, 0, nullptr)
        {
        }
    private:

        void NextBucket()
        {
            for (;BucketIndex < NumBuckets; BucketIndex++) {
                auto& bucket = MapRef.Bucket[BucketIndex];
                if (bucket.NodeList.IsEmpty())
                    continue;
                CurrListEntry = bucket.NodeList.Flink;
                break;
            }
        }

        HashMap &MapRef;
        size_t BucketIndex;
        ListEntry *CurrListEntry;

        friend class HashMap;
    };

    HashMap() {}

    virtual ~HashMap()
    {
        Clear();
    }

    bool Insert(const K& key, V&& value)
    {
        size_t i = Hash(key) % NumBuckets;
        auto& bucket = Bucket[i];
        for (auto listEntry = bucket.NodeList.Flink; listEntry != &bucket.NodeList; listEntry = listEntry->Flink) {
            auto node = CONTAINING_RECORD(listEntry, Node, Link);
            if (node->Key == key)
                return false;
        }

        auto node = new Node();
        if (node == nullptr)
            return false;

        node->Key = key;
        node->Value = Stdlib::Move(value);

        bucket.NodeList.InsertTail(&node->Link);
        return true;
    }

    bool Insert(const K& key, const V& value)
    {
        size_t i = Hash(key) % NumBuckets;
        auto& bucket = Bucket[i];
        for (auto listEntry = bucket.NodeList.Flink; listEntry != &bucket.NodeList; listEntry = listEntry->Flink) {
            auto node = CONTAINING_RECORD(listEntry, Node, Link);
            if (node->Key == key)
                return false;
        }

        auto node = new Node();
        if (node == nullptr)
            return false;

        node->Key = key;
        node->Value = value;
        if (node->Key != key || node->Value != value) {
            delete node;
            return false;
        }

        bucket.NodeList.InsertTail(&node->Link);
        return true;
    }

    bool Insert(K&& key, V&& value)
    {
        size_t i = Hash(key) % NumBuckets;
        auto& bucket = Bucket[i];
        for (auto listEntry = bucket.NodeList.Flink; listEntry != &bucket.NodeList; listEntry = listEntry->Flink) {
            auto node = CONTAINING_RECORD(listEntry, Node, Link);
            if (node->Key == key)
                return false;
        }

        auto node = new Node();
        if (node == nullptr)
            return false;

        node->Key = Stdlib::Move(key);
        node->Value = Stdlib::Move(value);

        bucket.NodeList.InsertTail(&node->Link);
        return true;
    }

    Iterator Lookup(const K& key)
    {
        size_t i = Hash(key) % NumBuckets;
        auto& bucket = Bucket[i];
        for (auto listEntry = bucket.NodeList.Flink; listEntry != &bucket.NodeList; listEntry = listEntry->Flink) {
            auto node = CONTAINING_RECORD(listEntry, Node, Link);
            if (node->Key == key) {
                return Iterator(*this, i, listEntry);
            }
        }
        return End();
    }
    
    bool Remove(const K& key)
    {
        size_t i = Hash(key) % NumBuckets;
        auto& bucket = Bucket[i];
        for (auto listEntry = bucket.NodeList.Flink; listEntry != &bucket.NodeList; listEntry = listEntry->Flink) {
            auto node = CONTAINING_RECORD(listEntry, Node, Link);
            if (node->Key == key) {
                node->Link.Remove();
                delete node;
                return true;
            }
        }

        return false;
    }

    void Clear()
    {
        for (size_t i = 0; i < NumBuckets; i++) {
            auto& bucket = Bucket[i];
            while (!bucket.NodeList.IsEmpty()) {
                auto listEntry = bucket.NodeList.RemoveHead();
                auto node = CONTAINING_RECORD(listEntry, Node, Link);
                delete node;
            }
        }
    }

    Iterator Begin()
    {
        auto it = Iterator(*this);
        it.NextBucket();
        return it;
    }

    Iterator End()
    {
        return Iterator(*this);
    }

private:
    HashMap(const HashMap& other) = delete;
    HashMap(HashMap&& other) = delete;
    HashMap& operator=(const HashMap& other) = delete;
    HashMap& operator=(HashMap&& other) = delete;

    struct Node final {
        Node() {}
        ~Node() {}
        ListEntry Link;
        K Key;
        V Value;
    private:
        Node(const Node& other) = delete;
        Node(Node&& other) = delete;
        Node& operator=(const Node& other) = delete;
        Node& operator=(Node&& other) = delete;
    };

    struct Bucket final {
        Bucket() {}
        ~Bucket() {}
        ListEntry NodeList;
    private:
        Bucket(const Bucket& other) = delete;
        Bucket(Bucket&& other) = delete;
        Bucket& operator=(const Bucket& other) = delete;
        Bucket& operator=(Bucket&& other) = delete;
    };

    Bucket Bucket[NumBuckets];
};

}