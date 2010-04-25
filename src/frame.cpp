#include "frame.hpp"
#include "code.hpp"
#include "method.hpp"
#include "scope.hpp"

using namespace pr;

Frame::Frame(Scope* scope, Frame* caller, Code* code)
:   Object(get_type()),
    scope(scope),
    caller(caller),
    code(code),
    state(READY),
    position(0),
    exc_handler(0),
    ret(0)
{
    assert(scope);
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
        //type->add_method("set", (Callable::mptr2)&Frame::set_);
        //type->add_method("set_local", (Callable::mptr2)&Frame::set_local_);
        type->add_method("execute", (Callable::mptr0)&Frame::execute);
        type->add_method("continue", (Callable::mptr1)&Frame::continue_);
        type->add_method("switch", (Callable::mptr1)&Frame::switch_);
        type->add_method("clone_continuation", (Callable::mptr0)&Frame::clone_continuation);
        type->add_method("set_exception_handler", (Callable::mptr1)&Frame::set_exception_handler_);
        type->add_method("get_exception_handler", (Callable::mptr0)&Frame::get_exception_handler_);
        type->add_method("set_scope", (Callable::mptr1)&Frame::set_scope_);
        type->add_method("scope", (Callable::mptr0)&Frame::scope_);
        type->add_method("caller", (Callable::mptr0)&Frame::caller_);
        type->add_method("code", (Callable::mptr0)&Frame::code_);
        type->add_method("state", (Callable::mptr0)&Frame::state_);
        //type->add_method("pollute", (Callable::mptr1)&Frame::pollute_);
        type->add_method("current_file", (Callable::mptr0)&Frame::current_file_);
        type->add_method("current_line", (Callable::mptr0)&Frame::current_line_);
    }
    return type;
}

void Frame::gc_mark()
{
    GC::mark(scope);
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
}

Frame* Frame::cast_frame()
{
    return this;
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

ObjP Frame::switch_(ObjP p)
{
    if (state == READY)
    {
        if (p != 0)
            throw new Exception("bad_argument", *this);

        state = IN_EXECUTION;
        get_executor()->set_frame(this);
        return error_object();
    }

    if (state == IN_EXECUTION)
    {
        push(p);
        get_executor()->set_frame(this);
        return error_object();
    }

    throw new Exception("switch_failed", *this);
}

Frame* Frame::clone_continuation()
{
    // TODO: state of the first cloned frame shouldn't be IN_CALL or FINISHED

    Frame* f = new Frame(scope.get(),
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

    return f;
}

ObjP Frame::set_scope_(ObjP p)
{
    Scope* s = cast_object<Scope*>(p);
    scope = s;
    return 0;
}

ObjP Frame::scope_()
{
    return scope ? inc_ref(*scope.get()) : 0;
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

#if 0
ObjP Frame::pollute_(ObjP obj)
{
    Type* t = pr::get_type(obj);
    Frame* f = this;

    std::map<Name, Callable>::iterator iter;
    for (iter = t->methods.begin(); iter != t->methods.end(); iter++)
        f->set_local(iter->first, *new Method(obj, iter->second));

    return 0;
}
#endif

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
