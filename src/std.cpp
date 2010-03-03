#include "std.hpp"
#include "type.hpp"
#include "primitives.hpp"
#include "list.hpp"
#include "gc.hpp"
#include "typedobject.hpp"
#include "assoc.hpp"
#include "string.hpp"
#include "frame.hpp"
#include "method.hpp"
#include "executor.hpp"
#include "code.hpp"
#include "file.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "ast.hpp"
#include <stdio.h>

using namespace pr;

Type* Std::type;

Std::Std()
:   Object(get_type())
{
}

Std::~Std()
{
}

Type* Std::get_type()
{
    if (!type)
    {
        type = new Type("std");
        type->add_method("print", (Callable::mptrx)&Std::print);
        type->add_method("struct", (Callable::mptr1)&Std::struct_);
        type->add_method("pollute", (Callable::mptr1)&Std::pollute);
        type->add_method("if", (Callable::mptrx)&Std::if_);
        type->add_method("gc", (Callable::mptr0)&Std::gc);
        type->add_method("while", (Callable::mptr2)&Std::while_);
        type->add_method("types", (Callable::mptr0)&Std::types);
        type->add_method("new_type", (Callable::mptr1)&Std::new_type_);
        type->add_method("apply", (Callable::mptr2)&Std::apply_);
        type->add_method("assoc", (Callable::mptr0)&Std::assoc_);
        type->add_method("try", (Callable::mptr2)&Std::try_);
        type->add_method("active_frame", (Callable::mptr0)&Std::caller_);
        type->add_method("repeat", (Callable::mptr1)&Std::repeat_);
        type->add_method("iter", (Callable::mptr2)&Std::iter_);
        type->add_method("call_with_cloned_frame", (Callable::mptr1)&Std::call_with_cloned_frame);
        type->add_method("new_continuation", (Callable::mptr1)&Std::new_continuation_);
        type->add_method("raise", (Callable::mptr1)&Std::raise_);
        type->add_method("compile_file", (Callable::mptr1)&Std::compile_file_);
        type->add_method("exception", (Callable::mptr2)&Std::exception_);
    }
    return type;
}

static void print_continuation0(MiniCode*, Frame* f)
{
    ObjP p = f->pop();
    String* s = to_string(p);
    printf("%s", s->get_data());
    dec_ref(p);
}

static void print_continuation1(MiniCode*, Frame*)
{
    printf("\n");
}

ObjP Std::print(List* l)
{
    static const MiniCode::Op ops[] =
    {
        PR_MC_PUSH(0)                // get item
        PR_MC_OP1(NextItemOrGoto, 5)
        PR_MC_CALL(1)                // 'to_string
        PR_MC_FUNC0()                // printf
        PR_MC_GOTO(0)                // repeat

        PR_MC_FUNC1()                // newline and return null
        PR_MC_NULL()
        PR_MC_END()
    };

    MiniCode* mc = new MiniCode(ops);
    mc->objects.push_back((ObjP)*l);
    mc->objects.push_back(name_to_symbol("to_string"));
    mc->func0 = print_continuation0;
    mc->func1 = print_continuation1;
    mc->counter = 0;

    get_executor()->emit(mc);

    return error_object();
}

ObjP Std::struct_(ObjP)
{
    return 0;
}

ObjP Std::pollute(ObjP obj)
{
    Type* t = pr::get_type(obj);
    Frame* f = get_executor()->get_caller_frame();

    std::map<Name, Callable>::iterator iter;
    for (iter = t->methods.begin(); iter != t->methods.end(); iter++)
        f->set_local(iter->first, *new Method(obj, iter->second));

    return 0;
}

ObjP Std::if_(List* l)
{
    int s = l->get_size();
    if (s != 2 && s != 3)
        throw new Exception(Name("bad_argument"), *l);

    ObjP pred = l->get(0);
    if (!is_bool(pred))
        throw new Exception(Name("bad_argument"), *l);

    ObjP ret;
    ret = is_true(pred) ? l->get(1) : ((l->get_size() == 3) ? l->get(2) : 0);

    if (ret == 0)
        return 0;

    deferred_method_call0(ret, "call");

    return error_object();
}

ObjP Std::gc()
{
    GC::force_gc();
    return 0;
}

ObjP Std::while_(ObjP a, ObjP b)
{
    static const MiniCode::Op ops[] =
    {
        PR_MC_PUSH(0)
        PR_MC_CALL_CALL()
        PR_MC_IF_FALSE(7)

        // 3
        PR_MC_PUSH(1)
        PR_MC_CALL_CALL()
        PR_MC_SET(2)      // store return value to slot 2
        PR_MC_GOTO(0)

        // 7
        PR_MC_PUSH(2)
        PR_MC_END()
    };

    MiniCode* mc = new MiniCode(ops);
    mc->objects.push_back(a);
    mc->objects.push_back(b);
    mc->objects.push_back(0);

    get_executor()->emit(mc);

    return error_object();
}

ObjP Std::types()
{
    List* l = new List();

    const std::set<Type*>& types = GC::get_types();

    std::set<Type*>::const_iterator iter;
    for (iter = types.begin(); iter != types.end(); iter++)
        l->append(**iter);

    return *l;
}

static ObjP new_typed_object(ObjP p, ObjP o)
{
    Type* t = get_type(p);
    return *new TypedObject(t, o);
}

ObjP Std::new_type_(ObjP p)
{
    if (!is_symbol(p))
        throw new Exception("bad_type", p);
    Type* t = new Type(symbol_to_name(p));
    GC::del_root(t);
    t->add_method("new", new_typed_object);
    t->add_method("unbox", (Callable::mptr0)&TypedObject::get_boxed_object_);
    t->add_method("box", (Callable::mptr1)&TypedObject::set_boxed_object_);
    return *new TypedObject(t, 0);
}

ObjP Std::apply_(ObjP p, ObjP a)
{
    deferred_method_callx(p, "call", to_list(a));
    return error_object();
}

ObjP Std::assoc_()
{
    return *new Assoc();
}

ObjP Std::try_(ObjP a, ObjP b)
{
    get_executor()->get_caller_frame();

    try
    {
        return method_call0(a, "call");
    }
    catch (Exception* e)
    {
        return method_call1(b, "call", *e);
    }
}

ObjP Std::caller_()
{
    return inc_ref(*get_executor()->get_caller_frame());
}

ObjP Std::repeat_(ObjP c)
{
    static const MiniCode::Op ops[] = {
        { MiniCode::Push, 0 },
        { MiniCode::Call, 1 },
        { MiniCode::IfTrue, 0 },
        { MiniCode::Push, 2 },
        { MiniCode::End, 0 },
    };

    MiniCode* mc = new MiniCode(ops);
    mc->objects.push_back(c);
    mc->objects.push_back(name_to_symbol("call"));
    mc->objects.push_back(false_object());

    get_executor()->emit(mc);

    return error_object();
}

ObjP Std::iter_(ObjP s, ObjP c)
{
    static const MiniCode::Op ops[] =
    {
        { MiniCode::Push, 0 },
        { MiniCode::Arg, -1 },
        { MiniCode::Push, 1 },
        { MiniCode::Call, 2 },

        { MiniCode::Peek, 1 },
        { MiniCode::IfFalse, 10 },
        { MiniCode::Peek, 1 },
        { MiniCode::IfNull, 10 },

        { MiniCode::Set, 0 },
        { MiniCode::Goto, 0 },

        { MiniCode::End, -1 },
    };

    MiniCode* mc = new MiniCode(ops);
    mc->objects.push_back(s);
    mc->objects.push_back(c);
    mc->objects.push_back(name_to_symbol("call"));

    executor->emit(mc);

    return error_object();
}

ObjP Std::call_with_cloned_frame(ObjP p)
{
    Frame* f = get_executor()->get_caller_frame();
    Frame* cf = f->clone_continuation();

    deferred_method_call1(p, "call", *cf);
    return error_object();
}

ObjP Std::new_continuation_(ObjP p)
{
    Frame* caller = get_executor()->get_caller_frame();
    Frame* f = new Frame(caller->previous.get(), 0, new Code());

    get_executor()->set_frame(f);

    deferred_method_call1(p, "call", *caller);

    return error_object();
}

ObjP Std::raise_(ObjP p)
{
    Exception* exc = is_object(p) ? to_object(p)->cast_exception() : 0;
    if (!exc)
        throw new Exception("bad_exception", p);

    throw exc;
}

ObjP Std::compile_file_(ObjP pp)
{
    String* fn = to_string(pp);

    File* f = new File(fn->get_data());

    String* s = f->read_file();

    Lexer* l = new Lexer(fn->get_data(), s);

    Parser* p = new Parser(l);

    AST* ast = p->get_ast();

    ast->debug_print();

    Code* code = new Code(ast, false);

    Frame* frame = new Frame(0, 0, code);

    return *frame;
}

ObjP Std::exception_(ObjP a, ObjP b)
{
    if (!is_symbol(a))
        throw new Exception("bad_type", a);
    return *new Exception(symbol_to_name(a), b);
}