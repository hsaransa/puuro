#include "callable.hpp"
#include "exception.hpp"
#include "list.hpp"
#include "primitives.hpp"
#include "gc.hpp"
#include <stdio.h>

using namespace pr;

static ObjP param(List* l, int i)
{
    return i < l->get_size() ? l->get(i) : 0;
}

ObjP Callable::callx(ObjP obj, List* l)
{
    PR_LOCAL_REF(obj);

    assert(type != NONE);

    switch (type)
    {
    case NONE:
        throw new Exception("internal_error", 0);
        return 0;
    case FUNCTION0:
        if (l->get_size() != 0)
            throw new Exception(Name("bad_argument_count"), *l);
        return func0(obj);
    case FUNCTION1:
        if (l->get_size() > 1)
            throw new Exception(Name("bad_argument_count"), *l);
        return func1(obj, param(l, 0));
    case FUNCTION2:
        if (l->get_size() > 2)
            throw new Exception(Name("bad_argument_count"), *l);
        return func2(obj, param(l, 0), param(l, 1));
    case FUNCTIONX:
        return funcx(obj, l);
    case METHOD0:
        if (l->get_size() != 0)
            throw new Exception(Name("bad_argument_count"), *l);
        return (to_object(obj)->*method0)();
    case METHOD1:
        if (l->get_size() > 1)
            throw new Exception(Name("bad_argument_count"), *l);
        return (to_object(obj)->*method1)(param(l, 0));
    case METHOD2:
        if (l->get_size() > 2)
            throw new Exception(Name("bad_argument_count"), *l);
        return (to_object(obj)->*method2)(param(l, 0), param(l, 1));
    case METHODX:
        return (to_object(obj)->*methodx)(l);
    case OBJECT:
        {
            List* ll = l->copy();
            PR_LOCAL_REF(ll);
            ll->prepend(obj);
            ObjP ret = method_callx(this->obj, Name("call"), ll);
            dec_ref(ll);
            return ret;
        }
    }

    assert(0);
}

ObjP Callable::call0(ObjP obj)
{
    PR_LOCAL_REF(obj);

    assert(type != NONE);

    switch (type)
    {
        case FUNCTION0: return func0(obj);
        case FUNCTION1: return func1(obj, 0);
        case FUNCTION2: return func2(obj, 0, 0);
        case FUNCTIONX: return funcx(obj, new List());
        case METHOD0: return (to_object(obj)->*method0)();
        case METHOD1: return (to_object(obj)->*method1)(0);
        case METHOD2: return (to_object(obj)->*method2)(0, 0);
        case METHODX: return (to_object(obj)->*methodx)(new List());
        case OBJECT:
            {
                ObjP ret = method_call1(this->obj, Name("call"), obj);
                return ret;
            }
        default:
            throw new Exception("bad_argument_count", 0);
    }
}

ObjP Callable::call1(ObjP obj, ObjP arg)
{
    PR_LOCAL_REF(obj);

    assert(type != NONE);

    switch (type)
    {
        case FUNCTION1: return func1(obj, arg);
        case FUNCTION2: return func2(obj, arg, 0);
        case FUNCTIONX: return funcx(obj, new List(1, arg));
        case METHOD1: return (((Object*)obj)->*method1)(arg);
        case METHOD2: return (((Object*)obj)->*method2)(arg, 0);
        case METHODX: return (((Object*)obj)->*methodx)(new List(1, arg));
        case OBJECT:
            {
                PR_LOCAL_REF(this->obj);
                ObjP ret = method_call2(this->obj, Name("call"), obj, arg);
                return ret;
            }
        default:
            throw new Exception("bad_argument_count", 0);
    }
}

ObjP Callable::call2(ObjP obj, ObjP arg, ObjP arg2)
{
    PR_LOCAL_REF(obj);

    assert(type != NONE);

    switch (type)
    {
        case FUNCTION2: return func2(obj, arg, arg2);
        case FUNCTIONX: return funcx(obj, new List(2, arg, arg2));
        case METHOD2: return (((Object*)obj)->*method2)(arg, arg2);
        case METHODX: return (((Object*)obj)->*methodx)(new List(2, arg, arg2));
        case OBJECT:
            {
                List* ll = new List(3, obj, arg, arg2);
                ObjP ret = method_callx(this->obj, Name("call"), ll);
                return ret;
            }
        default:
            throw new Exception("bad_argument_count", 0);
    }
}
