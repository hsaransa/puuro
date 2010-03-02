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

        String* read_file();

        virtual Type* get_type();

    private:
        std::string filename;
    };
}

#endif
