#ifndef _pr_primitives_hpp_
#define _pr_primitives_hpp_

#include "prdefs.hpp"
#include "object.hpp"
#include "nametable.hpp"
#include <limits.h>

namespace pr
{
    extern Type* null_type;
    extern Type* symbol_type;
    extern Type* fixnum_type;
    extern Type* bool_type;

    inline Object* to_object(ObjP p)
    {
        assert(p && (p & 3) == 0);
        return (Object*)p;
    }

    inline bool is_object(ObjP p)
    {
        return p && (p & 3) == 0;
    }

    inline bool is_fixnum(ObjP p)
    {
        return (p & 1) == 1;
    }

    inline bool is_symbol(ObjP p)
    {
        return p && (p & 15) == 2;
    }

    inline bool is_false(ObjP p)
    {
        return p == 6;
    }

    inline ObjP false_object() { return 6; }

    inline bool is_true(ObjP p)
    {
        return p == 10;
    }

    inline ObjP true_object() { return 10; }

    inline bool is_bool(ObjP p)
    {
        return is_false(p) || is_true(p);
    }

    inline ObjP error_object() { return 14; }

    inline bool is_error_object(ObjP p) { return p == 14; }

    inline Type* get_type(ObjP p)
    {
        if (!p)
            return null_type;
        else if (is_symbol(p))
            return symbol_type;
        else if (is_fixnum(p))
            return fixnum_type;
        else if (is_bool(p))
            return bool_type;
        else if (is_error_object(p))
        {
            assert(0);
            throw 0;
        }
        else
            return ((Object*)p)->get_type();
    }

    inline Name symbol_to_name(ObjP p)
    {
        assert(is_symbol(p));
        if ((p & 0x3) == 2)
            return Name(p >> 4);
        else
            return Name(0);
    }

    inline ObjP name_to_symbol(Name n)
    {
        assert(n.valid());
        return (n.id() << 4) | 2;
    }

#if 0
    inline ObjP member(ObjP p, Name n)
    {
        if (!p)
            return 0;
        if ((p & 0x3) == 2)
            return 0;
        return ((Object*)p)->member(n);
    }
#endif

    inline bool does_fit_fixnum(long long int i)
    {
        return i >= (INT_MIN / 2) && i <= (INT_MAX / 2);
    }

    inline int fixnum_to_int(ObjP p)
    {
        assert(is_fixnum(p));
        return (long)p >> 1;
    }

    inline ObjP int_to_fixnum(int i)
    {
        ObjP p = (i << 1) | 1;
        assert(fixnum_to_int(p) == i);
        return p;
    }

#if 0
    inline bool has_method(ObjP p, Name n)
    {
        return get_type(p)->get_method(n).type != Callable::NONE;
    }
#endif

    void init_primitive_types();
}

#endif
