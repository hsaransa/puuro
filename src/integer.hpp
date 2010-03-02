#ifndef _pr_integer_hpp_
#define _pr_integer_hpp_

#include "prdefs.hpp"
#include "object.hpp"
#include "primitives.hpp"
#include "exception.hpp"

namespace pr
{
    class Integer : public Object
    {
    public:
        Integer(const char* p);
        Integer(long long int value);
        virtual ~Integer();

        virtual Type* get_type();
        virtual Integer* cast_integer();

        int int_value();

    private:
        ObjP to_string_();
        ObjP add_(ObjP);
        ObjP sub_(ObjP);
        ObjP mul_(ObjP);
        ObjP div_(ObjP);
        ObjP mod_(ObjP);
        ObjP lt_(ObjP);
        ObjP eq_(ObjP);
        ObjP le_(ObjP);
        ObjP ge_(ObjP);
        ObjP gt_(ObjP);
        ObjP ne_(ObjP);
        ObjP abs_();
        ObjP neg_();
        ObjP is_true_();
        ObjP times_(ObjP);

        long long int value;

        static Type* type;
    };

    inline Integer* to_integer(ObjP p)
    {
        if (is_object(p))
            return to_object(p)->cast_integer();
        else
            if (is_fixnum(p))
                return new Integer(fixnum_to_int(p));
        throw new Exception("bad_integer", p);
    }

    inline int int_value(ObjP p)
    {
        if (is_object(p))
        {
            Integer* integer = to_object(p)->cast_integer();
            return integer->int_value();
        }
        else
            if (is_fixnum(p))
                return fixnum_to_int(p);
        throw new Exception("bad_integer", p);
    }
}

#endif
