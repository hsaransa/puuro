#ifndef _pr_exception_hpp_
#define _pr_exception_hpp_

#include "prdefs.hpp"
#include "nametable.hpp"
#include "object.hpp"

namespace pr
{
    class Exception : public Object
    {
    public:
        Exception(Name n, ObjP);
        virtual ~Exception();

        virtual Type* get_type();
        virtual void gc_mark();

        String* to_string();

    private:
        ObjP name_();
        ObjP obj_();

        Name name;
        ObjP obj;
    };

    static inline ObjP throw_exception(Exception* e)
    {
        throw e;
    }
}

#endif
