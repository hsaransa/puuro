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
        ObjP read_(ObjP);

        std::string filename;
        int fd;
        Name mode;
    };
}

#endif
