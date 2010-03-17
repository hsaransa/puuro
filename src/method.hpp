#ifndef _pr_method_hpp_
#define _pr_method_hpp_

#include "prdefs.hpp"
#include "type.hpp"

namespace pr
{
    class Method : public Object
    {
    public:
        // TODO: these two should really be different object types
        Method(ObjP object, Callable callable);
        Method(Type* t, Callable callable);
        virtual ~Method();

        virtual Type* get_type();
        virtual void gc_mark();

    private:
        ObjP to_string_();
        ObjP call_(List* arg);
        ObjP rcall_(List* arg);

    private:
        Ref<ObjP> object;
        Type* object_type;
        Callable callable;

        static Type* type;
    };
}

#endif
