#pragma once

#include <common/base.h>
#include <common/malloc/malloc.h>
#include <common/stdlib/pair.h>
#include "list_entry.h"

namespace Stdlib
{

template<typename K, size_t NumBuckets, size_t (*Hash)(const K& key)>
class HashSet
{
public:
    class Iterator
    {
    public:
        Iterator(HashSet& set)
            : set_(set)
            , bucketIndex(0)
            , currListEntry(nullptr)
        {
        }

        void NextBucket()
        {
            for (;bucketIndex < NumBuckets; bucketIndex++) {
                auto& bucket = set_.Bucket[bucketIndex];
                if (bucket.NodeList.IsEmpty())
                    continue;
                currListEntry = bucket.NodeList.Flink;
                break;
            }
        }

        void Next()
        {
            if (currListEntry != nullptr) {
                auto& bucket = set_.Bucket[bucketIndex];
                if (currListEntry->Flink != &bucket.NodeList)
                    currListEntry = currListEntry->Flink;
                else {
                    currListEntry = nullptr;
                    bucketIndex++;
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

        K& operator*() const
        {
            auto node = CONTAINING_RECORD(currListEntry, Node, Link);
            return node->Key;
        }

        bool operator !=(const Iterator &other) const
        {
            return (currListEntry != other.currListEntry);
        }

    private:
        HashSet &set_;
        size_t bucketIndex;
        ListEntry *currListEntry;
    };

    HashSet() : Count(0) {}

    virtual ~HashSet()
    {
        Clear();
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

    bool Add(K&& key)
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
        bucket.NodeList.InsertTail(&node->Link);
        Count++;
        return true;
    }

    bool Add(const K& key)
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
        bucket.NodeList.InsertTail(&node->Link);
        Count++;
        return true;
    }

    bool Exist(const K& key)
    {
        size_t i = Hash(key) % NumBuckets;
        auto& bucket = Bucket[i];
        for (auto listEntry = bucket.NodeList.Flink; listEntry != &bucket.NodeList; listEntry = listEntry->Flink) {
            auto node = CONTAINING_RECORD(listEntry, Node, Link);
            if (node->Key == key) {
                return true;
            }
        }
        return false;
    }

    bool Remove(const K& key)
    {
        size_t i = Hash(key) % NumBuckets;
        auto& bucket = Bucket[i];
        for (auto listEntry = bucket.NodeList.Flink; listEntry != &bucket.NodeList; listEntry = listEntry->Flink) {
            auto node = CONTAINING_RECORD(listEntry, Node, Link);
            if (node->Key == key) {
                node->Link.Remove();
                Count--;
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
                Count--;
                delete node;
            }
        }
    }

    size_t GetCount()
    {
        return Count;
    }

    HashSet& operator=(HashSet&& other)
    {
        if (this != &other) {
            Count = other.Count;
            other.Count = 0;
            for (size_t i = 0; i < NumBuckets; i++)
                Bucket[i] = Stdlib::Move(other.Bucket[i]);
        }
        return *this;
    }

    HashSet(HashSet&& other)
    {
        Count = other.Count;
        other.Count = 0;
        for (size_t i = 0; i < NumBuckets; i++)
            Bucket[i] = Stdlib::Move(other.Bucket[i]);        
    }

private:
    HashSet(const HashSet& other) = delete;
    HashSet& operator=(const HashSet& other) = delete;

    struct Node final {
        Node() {}
        ~Node() {}
        ListEntry Link;
        K Key;
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

        Bucket& operator=(Bucket&& other)
        {
            if (this != &other) {
                NodeList = Stdlib::Move(other.NodeList);
            }
            return *this;
        }

        Bucket(Bucket&& other)
        {
            NodeList = Stdlib::Move(other.NodeList);
        }

    private:
        Bucket(const Bucket& other) = delete;
        Bucket& operator=(const Bucket& other) = delete;
    };

    size_t Count;
    Bucket Bucket[NumBuckets];
};

}