#pragma once

#include <common/base.h>
#include <common/io.h>
#include <common/stdlib/string.h>
#include <common/stdlib/vector.h>
#include <common/stdlib/result.h>
#include <common/stdlib/error.h>

namespace FileSys
{

    struct DirectoryRecord
    {
        Stdlib::String file_name_;
        int type_;

        static const int kUnknown = -1;
        static const int kRegular = 1;
        static const int kDirectory = 2;

        DirectoryRecord()
            : type_(kUnknown)
        {
        }

        virtual ~DirectoryRecord()
        {
        }

        DirectoryRecord(DirectoryRecord&& other)
        {
            file_name_ = Stdlib::Move(other.file_name_);
            type_ = other.type_;
        }

        DirectoryRecord& operator=(DirectoryRecord&& other)
        {
            if (this != &other) {
                file_name_ = Stdlib::Move(other.file_name_);
                type_ = other.type_;                
            }
            return *this;
        }

    private:
        DirectoryRecord(const DirectoryRecord& other) = delete;
        DirectoryRecord& operator=(const DirectoryRecord& other) = delete;
    };

    class Directory
    {
    public:
        Directory();
        virtual ~Directory();

        Stdlib::Error Open(const char *path);
        Stdlib::Error Open(const Stdlib::String &path);

        void Close();
        Stdlib::Result<Stdlib::Vector<DirectoryRecord>, Stdlib::Error> List();

        const Stdlib::String& GetPath() const;

    private:
        Directory(const Directory& other) = delete;
        Directory(Directory&& other) = delete;
        Directory& operator=(const Directory& other) = delete;
        Directory& operator=(Directory&& other) = delete;

        Stdlib::String path_;
        void* dir_handle_;
    };
}