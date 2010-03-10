#ifndef _pr_file_hpp_
#define _pr_file_hpp_

#include "prdefs.hpp"
#include "object.hpp"
#include "nametable.hpp"
#include <string>

namespace pr
{
    class File : public Object
    {
    public:
        File();
        File(const std::string& fn, int fd, Name mode);
        virtual ~File();

        virtual Type* get_type();

        void set_filename(const char*);
        void open(Name mode);
        void close();

        String* read();

        int get_fd() { return fd; }

    private:
        ObjP set_filename_(ObjP);
        ObjP open_(ObjP);
        ObjP socket_(ObjP, ObjP);
        ObjP connect_(ObjP);
        ObjP bind_(ObjP, ObjP);
        ObjP read_(ObjP);
        ObjP write_(ObjP);
        ObjP close_();

        int fd;

        std::string filename;
        Name file_mode;

        Name sock_domain;
        Name sock_type;
    };
}

#endif
