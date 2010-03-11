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
    state(READY),
    position(0),
    exc_handler(0),
    ret(0)
{
    assert(code);
}

Frame::~Frame()
{
    std::list<MiniCode*>::iterator iter;
    for (iter = minicode_stack.begin(); iter != minicode_stack.end(); iter++)
        delete *iter;
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
        type->add_method("set_exception_handler", (Callable::mptr1)&Frame::set_exception_handler_);
        type->add_method("get_exception_handler", (Callable::mptr0)&Frame::get_exception_handler_);
        type->add_method("previous", (Callable::mptr0)&Frame::previous_);
        type->add_method("caller", (Callable::mptr0)&Frame::caller_);
        type->add_method("code", (Callable::mptr0)&Frame::code_);
        type->add_method("state", (Callable::mptr0)&Frame::state_);
        type->add_method("cut_previous", (Callable::mptr0)&Frame::cut_previous_);
        type->add_method("pollute", (Callable::mptr1)&Frame::pollute_);
        type->add_method("current_file", (Callable::mptr0)&Frame::current_file_);
        type->add_method("current_line", (Callable::mptr0)&Frame::current_line_);
    }
    return type;
}

void Frame::gc_mark()
{
    if (previous)
        GC::mark(previous);
    if (caller)
        GC::mark(caller);
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

    std::map<Name, Ref<ObjP> >::iterator iter;
    for (iter = locals.begin(); iter != locals.end(); iter++)
        GC::mark(iter->second);
}

Frame* Frame::cast_frame()
{
    return this;
}

void Frame::set_local(Name n, ObjP p)
{
    locals[n] = p;
}

void Frame::set(Name n, ObjP p)
{
    Frame* f = this;

    // Get the frame that has this variable assigned.

    while (f)
    {
        if (f->locals.count(n))
        {
            // Replace the old variable.
            f->locals[n] = p;
            return;
        }
        else
            f = f->previous.get();
    }

    // Not found, add variable to this frame.

    locals[n] = p;
}

ObjP Frame::set_local_(ObjP n, ObjP v)
{
    if (!is_symbol(n))
        throw new Exception("bad_argument", n);
    set_local(symbol_to_name(n), v);
    return inc_ref(v);
}

ObjP Frame::set_(ObjP n, ObjP v)
{
    if (!is_symbol(n))
        throw new Exception("bad_argument", n);
    set(symbol_to_name(n), v);
    return inc_ref(v);
}

ObjP Frame::set_exception_handler_(ObjP e)
{
    ObjP prev = inc_ref(exc_handler);
    exc_handler = e;
    return prev;
}

ObjP Frame::get_exception_handler_()
{
    Frame* f = this;
    while (f && f->exc_handler == 0)
        f = f->caller.get();
    return f ? inc_ref(f->exc_handler.get()) : 0;
}

void Frame::push(ObjP p)
{
    stack.push_back(p);
}

ObjP Frame::pop()
{
    assert(!stack.empty());
    ObjP p = stack[stack.size()-1];
    inc_ref(p);
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
        return inc_ref(locals[n]);
}

ObjP Frame::execute()
{
    if (state != READY)
        throw new Exception("execute_failed", *this);

    get_executor()->call(this);
    return error_object();
}

ObjP Frame::continue_(ObjP p)
{
    // TODO: strictly speaking, frame can only be continued if
    // it is IN_EXECUTION

    if (state != READY && state != IN_EXECUTION)
        throw new Exception("continue_failed", *this);

    if (state == READY)
    {
        get_executor()->call(this);
        return error_object();
    }

    state = IN_EXECUTION;

    push(p);

    get_executor()->set_frame(this);
    return error_object();
}

Frame* Frame::clone_continuation()
{
    // TODO: state of the first cloned frame shouldn't be IN_CALL or FINISHED

    Frame* f = new Frame(previous.get(),
                         caller.get() ? caller->clone_continuation() : 0,
                         code.get());

    f->position = position;
    f->state = state;

    std::list<MiniCode*>::iterator iter;
    for (iter = minicode_stack.begin(); iter != minicode_stack.end(); iter++)
        f->minicode_stack.push_back((*iter)->clone());

    f->stack = stack;
    f->args = args;
    f->exc_handler = exc_handler;
    f->ret = ret;
    f->locals = locals;

    return f;
}

ObjP Frame::previous_()
{
    return previous ? inc_ref(*previous.get()) : 0;
}

ObjP Frame::caller_()
{
    return caller ? inc_ref(*caller.get()) : 0;
}

ObjP Frame::code_()
{
    return code ? inc_ref(*code.get()) : 0;
}

ObjP Frame::state_()
{
    switch (state)
    {
    case READY:        return name_to_symbol("ready");
    case IN_EXECUTION: return name_to_symbol("in_execution");
    case IN_CALL:      return name_to_symbol("in_call");
    case FINISHED:     return name_to_symbol("finished");
    }
    return 0;
}

ObjP Frame::cut_previous_()
{
    previous = 0;
    return 0;
}

ObjP Frame::pollute_(ObjP obj)
{
    Type* t = pr::get_type(obj);
    Frame* f = this;

    std::map<Name, Callable>::iterator iter;
    for (iter = t->methods.begin(); iter != t->methods.end(); iter++)
        f->set_local(iter->first, *new Method(obj, iter->second));

    return 0;
}

const char* Frame::get_current_file()
{
    if (state == FINISHED)
        return 0;
    return Name(code->get_position(position).file).s();
}

int Frame::get_current_line()
{
    if (state == FINISHED)
        return 0;
    return code->get_position(position).line;
}

ObjP Frame::current_file_()
{
    if (state == FINISHED)
        return 0;
    return name_to_symbol(Name(code->get_position(position).file));
}

ObjP Frame::current_line_()
{
    if (state == FINISHED)
        return 0;
    return int_to_fixnum(code->get_position(position).line);
}
