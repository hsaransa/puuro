#include "exception.hpp"
#include "string.hpp"
#include "primitives.hpp"
#include "gc.hpp"

using namespace pr;

Exception::Exception(Name n, ObjP obj)
:   Object(get_type()),
    name(n),
    obj(obj)
{
}

Exception::~Exception()
{
}

Type* Exception::get_type()
{
    static Type* type;
    if (!type)
    {
        type = new Type(Name("exception"));
        type->add_method("to_string", (Callable::mptr0)&Exception::to_string);
    }
    return type;
}

void Exception::gc_mark()
{
    GC::mark(obj);
}

String* Exception::to_string()
{
    String* s = new String(name.s());
    PR_LOCAL_REF(s);
    String* s2 = call_to_string(obj);
    s->append(": ");
    s->append(s2);
    return s;
}
