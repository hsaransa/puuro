#ifndef _pr_assoc_hpp_
#define _pr_assoc_hpp_

#include "prdefs.hpp"
#include "object.hpp"
#include "gc.hpp"

namespace pr
{
    class Assoc : public Object
    {
    public:
        Assoc();
        virtual ~Assoc();

        virtual Type* get_type();
        virtual void gc_mark();

    private:
        ObjP to_string_();
        ObjP get_(ObjP);
        ObjP set_(ObjP, ObjP);
        ObjP copy_();

        std::map<Name, Ref<ObjP> > assoc;
    };
}

#endif
