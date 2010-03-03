#include "integer.hpp"
#include "type.hpp"
#include "string.hpp"
#include "primitives.hpp"
#include "minicode.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <cmath>
#include <limits.h>

using namespace pr;

Type* Integer::type;

Integer::Integer(const char* p)
:   Object(get_type())
{
    value = atoll(p);
}

Integer::Integer(long long int v)
:   Object(get_type()), value(v)
{
}

Integer::~Integer()
{
}

int Integer::int_value()
{
    if (value <= INT_MIN || value >= INT_MAX)
        throw new Exception("int_overflow", *this);
    return (int)value;
}

Type* Integer::get_type()
{
    if (!type)
    {
        type = new Type("integer");
        type->add_method("to_string", (Callable::mptr0)&Integer::to_string_);
        type->add_method("add", (Callable::mptr1)&Integer::add_);
        type->add_method("sub", (Callable::mptr1)&Integer::sub_);
        type->add_method("mul", (Callable::mptr1)&Integer::mul_);
        type->add_method("div", (Callable::mptr1)&Integer::div_);
        type->add_method("mod", (Callable::mptr1)&Integer::mod_);
        type->add_method("lt", (Callable::mptr1)&Integer::lt_);
        type->add_method("le", (Callable::mptr1)&Integer::le_);
        type->add_method("eq", (Callable::mptr1)&Integer::eq_);
        type->add_method("ne", (Callable::mptr1)&Integer::ne_);
        type->add_method("gt", (Callable::mptr1)&Integer::gt_);
        type->add_method("ge", (Callable::mptr1)&Integer::ge_);
        type->add_method("abs", (Callable::mptr0)&Integer::abs_);
        type->add_method("neg", (Callable::mptr0)&Integer::neg_);
        type->add_method("is_true", (Callable::mptr1)&Integer::is_true_);
        type->add_method("times", (Callable::mptr1)&Integer::times_);
    }
    return type;
}

Integer* Integer::cast_integer()
{
    return this;
}

ObjP Integer::to_string_()
{
    char buf[64];
    snprintf(buf, sizeof(buf), "%lld", value);
    return *new String(buf);
}

ObjP Integer::add_(ObjP p)
{
    Integer* i = pr::to_integer(p);
    return *new Integer(value + i->value);
}

ObjP Integer::sub_(ObjP p)
{
    Integer* i = pr::to_integer(p);
    return *new Integer(value - i->value);
}

ObjP Integer::mul_(ObjP p)
{
    Integer* i = pr::to_integer(p);
    return *new Integer(value * i->value);
}

ObjP Integer::div_(ObjP p)
{
    Integer* i = pr::to_integer(p);
    if (i->value == 0)
        throw new Exception("division_by_zero", p);
    if ((value % i->value) != 0)
        throw new Exception("not_divisible", p);
    return *new Integer(value / i->value);
}

ObjP Integer::mod_(ObjP p)
{
    Integer* i = pr::to_integer(p);
    return *new Integer(value % i->value);
}

ObjP Integer::lt_(ObjP p)
{
    Integer* i = pr::to_integer(p);
    return value < i->value ? true_object() : false_object();
}

ObjP Integer::le_(ObjP p)
{
    Integer* i = pr::to_integer(p);
    return value <= i->value ? true_object() : false_object();
}

ObjP Integer::eq_(ObjP p)
{
    Integer* i = pr::to_integer(p);
    return value == i->value ? true_object() : false_object();
}

ObjP Integer::ne_(ObjP p)
{
    Integer* i = pr::to_integer(p);
    return value != i->value ? true_object() : false_object();
}

ObjP Integer::ge_(ObjP p)
{
    Integer* i = pr::to_integer(p);
    return value >= i->value ? true_object() : false_object();
}

ObjP Integer::gt_(ObjP p)
{
    Integer* i = pr::to_integer(p);
    return value > i->value ? true_object() : false_object();
}

ObjP Integer::abs_()
{
    return *new Integer(::abs(value));
}

ObjP Integer::neg_()
{
    return *new Integer(-value);
}

ObjP Integer::is_true_()
{
    return value ? true_object() : false_object();
}

ObjP Integer::times_(ObjP p)
{
    static const MiniCode::Op ops[] =
    {
        PR_MC_PUSH(0)
        PR_MC_OP1(IfCounterGE, 7)
        PR_MC_OP0(IncCounter)

        // 3
        PR_MC_PUSH(1)
        PR_MC_CALL_CALL()
        PR_MC_SET(2)
        PR_MC_GOTO(0)

        // 7
        PR_MC_PUSH(2)
        PR_MC_END()
    };

    MiniCode* mc = new MiniCode(ops);
    mc->objects.push_back((ObjP)*this);
    mc->objects.push_back(p);
    mc->objects.push_back(0);
    mc->counter = 0;

    executor->emit(mc);

    return error_object();
}
