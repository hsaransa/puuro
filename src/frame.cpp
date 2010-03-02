#include "frame.hpp"
#include "code.hpp"
#include "primitives.hpp"
#include "type.hpp"
#include "list.hpp"
#include "closure.hpp"
#include "method.hpp"
#include "gc.hpp"
#include "string.hpp"
#include "executor.hpp"
#include <stdio.h>

using namespace pr;

Frame::Frame(Frame* previous, Frame* caller, Code* code)
:   Object(get_type()),
    previous(previous),
    caller(caller),
    code(code),
    position(0),
    exc_handler(0),
    ret(0),
    in_execution(false)
{
    assert(code);
}

Frame::~Frame()
{
}

ObjP Frame::to_string_()
{
    return *new String("<frame>");
}

Type* Frame::get_type()
{
    static Type* type;
    if (!type)
    {
        type = new Type("frame");
        type->add_method("to_string", (Callable::mptr0)&Frame::to_string_);
        type->add_method("set", (Callable::mptr2)&Frame::set_);
        type->add_method("set_local", (Callable::mptr2)&Frame::set_local_);
        type->add_method("execute", (Callable::mptr0)&Frame::execute);
        type->add_method("continue", (Callable::mptr1)&Frame::continue_);
        type->add_method("clone_continuation", (Callable::mptr0)&Frame::clone_continuation);
        type->add_method("set_exception_handler", (Callable::mptr1)&Frame::set_exception_handler);
    }
    return type;
}

void Frame::gc_mark()
{
    if (previous)
        GC::mark(previous);
    if (caller)
        GC::mark(caller);
    if (callee)
        GC::mark(callee);
    GC::mark(code);

    for (int i = 0; i < (int)stack.size(); i++)
        GC::mark(stack[i]);

    for (int i = 0; i < (int)args.size(); i++)
        GC::mark(args[i]);

    GC::mark(exc_handler);

    GC::mark(ret);

    {
        std::list<MiniCode*>::iterator iter;
        for (iter = minicode_stack.begin(); iter != minicode_stack.end(); iter++)
            (*iter)->gc_mark();
    }

    std::map<Name, ObjP>::iterator iter;
    for (iter = locals.begin(); iter != locals.end(); iter++)
        GC::mark(iter->second);
}

void Frame::set_local(Name n, ObjP p)
{
    locals[n] = p;
}

void Frame::set(Name n, ObjP p)
{
    Frame* f = this;

    while (f)
    {
        if (f->locals.count(n))
        {
            f->locals[n] = p;
            return;
        }
        else
            f = f->previous.get();
    }

    locals[n] = p;
}

ObjP Frame::set_local_(ObjP n, ObjP v)
{
    if (!is_symbol(n))
        throw new Exception("bad_argument", n);
    set_local(symbol_to_name(n), v);
    return v;
}

ObjP Frame::set_(ObjP n, ObjP v)
{
    if (!is_symbol(n))
        throw new Exception("bad_argument", n);
    set(symbol_to_name(n), v);
    return v;
}

ObjP Frame::set_exception_handler(ObjP e)
{
    exc_handler = e;
    return 0;
}

void Frame::push(ObjP p)
{
    inc_ref(p);
    stack.push_back(p);
}

ObjP Frame::pop()
{
    assert(!stack.empty());
    ObjP p = stack[stack.size()-1];
    stack.erase(stack.begin() + (stack.size() - 1));
    return p;
}

void Frame::arg(ObjP p)
{
    args.push_back(p);
}

ObjP Frame::lookup(Name n)
{
    if (locals.count(n) == 0)
    {
        if (previous)
            return previous->lookup(n);
        throw new Exception("not_defined", name_to_symbol(n));
    }
    else
        return locals[n];
}

ObjP Frame::execute()
{
    if (in_execution)
        throw new Exception("execute_failed", *this);

    get_executor()->call(this);
    return error_object();
}

ObjP Frame::continue_(ObjP p)
{
//    if (!in_execution)
//        throw new Exception("continue_failed", *this);

    Frame* f = this;
    while (f->callee)
        f = f->callee.get();

    f->push(p);
    get_executor()->set_frame(f);

    return error_object();
}

Frame* Frame::clone_continuation()
{
    Frame* f = new Frame(previous.get(),
                         caller.get() ? caller->clone_continuation() : 0,
                         code.get());

    f->position = position;

    std::list<MiniCode*>::iterator iter;
    for (iter = minicode_stack.begin(); iter != minicode_stack.end(); iter++)
        f->minicode_stack.push_back((*iter)->clone());

    f->stack = stack;
    f->args = args;
    f->ret = ret;
    f->in_execution = in_execution;
    f->locals = locals;

    return f;
}
