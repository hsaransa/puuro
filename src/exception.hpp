#ifndef _pr_exception_hpp_
#define _pr_exception_hpp_

#include "prdefs.hpp"
#include "primitives.hpp"

namespace pr
{
    class Exception : public Object
    {
    public:
        Exception(Name n, ObjP);
        virtual ~Exception();

        virtual Type* get_type();
        virtual void gc_mark();

        virtual Exception* cast_exception();

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

    template<typename T>
    T cast_object(ObjP p)
    {
        if (!is_object(p))
            throw new Exception("bad_type", p);
        T t = dynamic_cast<T>(to_object(p));
        if (!t)
            throw new Exception("bad_type", p);
        return t;
    }
}

#endif
