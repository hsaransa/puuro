#include "primitives.hpp"
#include "string.hpp"
#include "type.hpp"
#include "integer.hpp"
#include "list.hpp"
#include "executor.hpp"
#include "minicode.hpp"
#include <stdio.h>

using namespace pr;

namespace pr
{
    Type* null_type;
    Type* symbol_type;
    Type* fixnum_type;
    Type* bool_type;
}

/* Null methods. */

static ObjP null_s(ObjP p)
{
    assert(!p);
    return *new String("null");
}

/* Symbol methods. */

static ObjP symbol_s(ObjP p)
{
    assert(is_symbol(p));
    String* s = new String("'");
    s->append(symbol_to_name(p).s());
    return *s;
}

static ObjP symbol_as_string(ObjP p)
{
    assert(is_symbol(p));
    String* s = new String(symbol_to_name(p).s());
    return *s;
}

static ObjP symbol_eq(ObjP a, ObjP b)
{
    return a == b ? true_object() : false_object();
}

/* Fixnum methods. */

#if 0
static ObjP fixnum_add(ObjP a, ObjP b)
{
    return *new Integer(fixnum_to_int(a))->add(b);
}
#endif

static ObjP fixnum_s(ObjP p)
{
    char buf[32];
    snprintf(buf, 32, "%d", fixnum_to_int(p));
    return *new String(buf);
}

static ObjP fixnum_bad_method(ObjP p, ObjP arg1, ObjP arg2)
{
    if (!is_symbol(arg1))
        throw new Exception(Name("bad_argument"), arg1);
    if (!to_list(arg2))
        throw new Exception(Name("bad_argument"), arg2);

    Integer* obj = new Integer(fixnum_to_int(p));

    deferred_method_callx(*obj, symbol_to_name(arg1), to_list(arg2));

    return error_object();
}

static ObjP bool_s(ObjP p)
{
    assert(is_bool(p));
    if (is_true(p))
        return *new String("true");
    else
        return *new String("false");
}

static ObjP bool_and(ObjP a, ObjP b)
{
    assert(is_bool(a) && is_bool(b));
    return a == true_object() && b == true_object() ? true_object() : false_object();
}

static ObjP bool_or(ObjP a, ObjP b)
{
    assert(is_bool(a) && is_bool(b));
    return a == true_object() || b == true_object() ? true_object() : false_object();
}
static ObjP bool_not(ObjP p)
{
    assert(is_bool(p));
    return is_true(p) ? false_object() : true_object();
}

static ObjP bool_if(ObjP a, ObjP b, ObjP c)
{
    deferred_method_call0(a == true_object() ? b : c, "call");
    return error_object();
}

static ObjP bool_then(ObjP a, ObjP b)
{
    assert(is_bool(a));

    if (a != true_object())
        return a;

    deferred_method_call0(b, "call");
    return error_object();
}

static ObjP bool_else(ObjP a, ObjP b)
{
    assert(is_bool(a));

    if (a != false_object())
        return a;

    deferred_method_call0(b, "call");
    return error_object();
}

static ObjP bool_pick(ObjP a, ObjP b, ObjP c)
{
    assert(is_bool(a));
    return a == true_object() ? b : c;
}

void pr::init_primitive_types()
{
    null_type = new Type("null");
    GC::add_root(null_type);
    null_type->add_method("to_string", null_s);

    symbol_type = new Type("symbol");
    GC::add_root(symbol_type);
    symbol_type->add_method("to_string", symbol_s);
    symbol_type->add_method("s", symbol_as_string);
    symbol_type->add_method("eq", symbol_eq);

    fixnum_type = new Type("fixnum");
    GC::add_root(fixnum_type);
//    fixnum_type->add_method("add", fixnum_add);
    fixnum_type->add_method("to_string", fixnum_s);
    fixnum_type->add_method("bad_method", fixnum_bad_method);

    bool_type = new Type("bool");
    GC::add_root(bool_type);
    bool_type->add_method("and", bool_and);
    bool_type->add_method("or", bool_or);
    bool_type->add_method("not", bool_not);
    bool_type->add_method("if", bool_if);
    bool_type->add_method("then", bool_then);
    bool_type->add_method("call", bool_then);
    bool_type->add_method("else", bool_else);
    bool_type->add_method("pick", bool_pick);
    bool_type->add_method("to_string", bool_s);
}
