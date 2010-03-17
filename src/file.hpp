#ifndef _pr_file_hpp_
#define _pr_file_hpp_

#include "prdefs.hpp"
#include "object.hpp"
#include "nametable.hpp"

namespace pr
{
    class File : public Object
    {
    public:
        File();
        File(int fd);
        virtual ~File();

        virtual Type* get_type();

        void set_filename(const char*);
        void open(Name mode);
        void close();

        String* read();

        int get_fd() { return fd; }

    private:
        ObjP to_string_();
        ObjP set_filename_(ObjP);
        ObjP open_(ObjP);
        ObjP socket_(ObjP, ObjP);
        ObjP connect_(ObjP);
        ObjP bind_(ObjP);
        ObjP listen_(ObjP);
        ObjP accept_();
        ObjP read_(ObjP);
        ObjP write_(ObjP);
        ObjP close_();
        ObjP setsockopt_(ObjP, ObjP);

        int fd;

        std::string filename;
        Name file_mode;

        Name sock_domain;
        Name sock_type;
    };
}

#endif
