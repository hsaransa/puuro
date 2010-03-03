#ifndef _pr_file_hpp_
#define _pr_file_hpp_

#include "prdefs.hpp"
#include "object.hpp"
#include <string>

namespace pr
{
    class File : public Object
    {
    public:
        File(const char* fn);
        virtual ~File();

        virtual Type* get_type();

        String* read_file();

    private:
        std::string filename;
    };
}

#endif
