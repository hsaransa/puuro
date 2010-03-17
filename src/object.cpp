#include "exception.hpp"
#include "gc.hpp"

using namespace pr;

Object::Object(Type* t)
:   ref_count(1 << GC_BITS)
{
    assert(t);
    GC::register_instance(t, this);
    GC::mark_alive(this);
}

Object::~Object()
{
}

void Object::gc_mark()
{
}

int Object::mem_usage()
{
    return sizeof(Object);
}

void Object::destroy()
{
    GC::destroy_instance(get_type(), this);
}

Integer* Object::cast_integer()
{
    throw new Exception("bad_type", *this);
}

Code* Object::cast_code()
{
    throw new Exception("bad_type", *this);
}

List* Object::cast_list()
{
    throw new Exception("bad_type", *this);
}

String* Object::cast_string()
{
    throw new Exception("bad_type", *this);
}

Exception* Object::cast_exception()
{
    throw new Exception("bad_type", *this);
}

Frame* Object::cast_frame()
{
    throw new Exception("bad_type", *this);
}
