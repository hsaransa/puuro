#include "string.hpp"
#include "type.hpp"
#include "primitives.hpp"
#include "gc.hpp"
#include "integer.hpp"
#include <stdio.h>

using namespace pr;

String::String()
:   Object(get_type())
{
}

String::String(const char* p)
:   Object(get_type())
{
    data = p;
}

String::String(const char* p, int size)
:   Object(get_type())
{
    data = std::string(p, p+size);
}

String::~String()
{
}

Type* String::get_type()
{
    static Type* type;
    if (!type)
    {
        type = new Type("string");
        type->add_method("to_string", (Callable::mptr0)&String::copy);
        type->add_method("add", (Callable::mptr1)&String::add);
        type->add_method("at", (Callable::mptr1)&String::at);
        type->add_method("len", (Callable::mptr0)&String::len_);
        type->add_method("eq", (Callable::mptr1)&String::eq_);
        type->add_method("lt", (Callable::mptr1)&String::lt_);
        type->add_method("to_symbol", (Callable::mptr0)&String::to_symbol_);
        type->add_method("find", (Callable::mptr1)&String::find_);
        type->add_method("all_before", (Callable::mptr1)&String::all_before_);
        type->add_method("all_after", (Callable::mptr1)&String::all_after_);
        type->add_method("to_integer", (Callable::mptr1)&String::to_integer_);
    }

    return type;
}

String* String::cast_string()
{
    return this;
}

String* String::copy() const
{
    String* str = new String();
    str->data = data;
    return str;
}

const char* String::get_data() const
{
    return data.c_str();
}

int String::get_size() const
{
    return (int)data.length();
}

void String::append(String* s)
{
    assert(s);
    data += s->data;
}

void String::append(const char* s)
{
    assert(s);
    data += s;
}

ObjP String::add(ObjP p)
{
    String* s = copy();
    s->append(to_string(p));
    return *s;
}

ObjP String::at(ObjP p)
{
    int v = int_value(p);
    if (v < 0 || v >= (int)data.length())
        throw new Exception("out_of_range", p);
    char buf[2];
    buf[0] = data[v];
    buf[1] = '\0';
    return *new String(buf);
}

ObjP String::len_()
{
    return int_to_fixnum(data.length());
}

ObjP String::eq_(ObjP p)
{
    String* s = to_string(p);
    return s->data == data ? true_object() : false_object();
}

ObjP String::lt_(ObjP p)
{
    String* s = to_string(p);
    return s->data < data ? true_object() : false_object();
}

ObjP String::to_symbol_()
{
    return name_to_symbol(Name(data));
}

ObjP String::find_(ObjP p)
{
    String* s2 = to_string(p);
    int end = (int)data.length() - (int)s2->get_size() + 1;

    for (int i = 0; i < end; i++)
    {
        int j;
        for (j = 0; j < s2->get_size(); j++)
            if (data[i+j] != s2->get_data()[j])
                break;

        if (j == s2->get_size())
            return int_to_fixnum(i);
    }

    return int_to_fixnum(-1);
}

ObjP String::all_before_(ObjP p)
{
    int i = int_value(p);
    if (i < 0 || i > (int)data.length())
        throw new Exception("out_of_range", p);

    String* s2 = new String(data.c_str(), i);
    return *s2;
}

ObjP String::all_after_(ObjP p)
{
    int i = int_value(p);
    if (i < 0 || i > (int)data.length())
        throw new Exception("out_of_range", p);

    String* s2 = new String(data.c_str()+i, (int)data.length()-i);
    return *s2;
}

ObjP String::to_integer_()
{
    return *new Integer(data.c_str());
}
