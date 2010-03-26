#ifndef _pr_float_hpp_
#define _pr_float_hpp_

#include "prdefs.hpp"
#include "object.hpp"
#include "exception.hpp"
#include "integer.hpp"

namespace pr
{
    class Float : public Object
    {
    public:
        Float(double v);
        Float(const char* f);
        virtual ~Float();

        virtual Type* get_type();

        inline double get_value() const { return value; }

    private:
        ObjP to_string_();
        ObjP add_(ObjP);
        ObjP sqrt_();

        double value;
    };

    inline double float_value(ObjP p)
    {
        if (is_fixnum(p))
            return (double)fixnum_to_int(p);

        Integer* i = try_cast_object<Integer*>(p);
        if (i)
            return (double)i->get_value();

        return cast_object<Float*>(p)->get_value();
    }
}

#endif
