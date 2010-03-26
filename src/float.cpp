#include "float.hpp"
#include "string.hpp"
#include "type.hpp"
#include <stdio.h>
#include <cmath>

using namespace pr;

Float::Float(double v)
:   Object(get_type()), value(v)
{
}

Float::~Float()
{
}

Type* Float::get_type()
{
    static Type* type;
    if (!type)
    {
        type = new Type("float");
        type->add_method("to_string", (Callable::mptr0)&Float::to_string_);
        type->add_method("add", (Callable::mptr1)&Float::add_);
        type->add_method("sqrt", (Callable::mptr0)&Float::sqrt_);
    }
    return type;
}

ObjP Float::to_string_()
{
    char buf[64];
    snprintf(buf, sizeof(buf), "%f", value);
    return *new String(buf);
}

ObjP Float::add_(ObjP p)
{
    double v = float_value(p);
    return *new Float(value + v);
}

ObjP Float::sqrt_()
{
    if (value < 0)
        throw new Exception("bad_value", *this);
    return *new Float(std::sqrt(value));
}
