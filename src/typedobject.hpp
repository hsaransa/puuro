#ifndef _pr_typedobject_hpp_
#define _pr_typedobject_hpp_

#include "prdefs.hpp"
#include "object.hpp"

namespace pr
{
    class TypedObject : public Object
    {
    public:
        TypedObject(Type*, ObjP o);
        virtual ~TypedObject();

        virtual Type* get_type();
        virtual void gc_mark();

        ObjP get_boxed_object_();
        ObjP set_boxed_object_(ObjP);

    private:
        Type* type;
        ObjP object;
    };
}

#endif
