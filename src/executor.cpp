#include "executor.hpp"
#include "frame.hpp"
#include "code.hpp"
#include "method.hpp"
#include "closure.hpp"
#include "list.hpp"
#include "minicode.hpp"
#include "integer.hpp"
#include <stdio.h>

using namespace pr;

Executor* pr::executor;

Executor::Executor()
:   Object(get_type())
{
}

Executor::~Executor()
{
}

Type* Executor::get_type()
{
    static Type* type;
    if (!type)
        type = new Type("executor");
    return type;
}

void Executor::gc_mark()
{
}

void Executor::emit(MiniCode* mc)
{
    assert(mc);
    f->minicode_stack.push_back(mc);
}

void Executor::set_frame(Frame* f)
{
    assert(!f->callee);
    this->f = f;
}

void Executor::call(Frame* nf)
{
    f->callee = nf;
    nf->caller = f;
    f = nf;
}

void Executor::execute()
{
    assert(!f->in_execution);
    //if (f->in_execution)
    //    throw new Exception(Name("already_in_execution"), *this);

    f->in_execution = true;
    f->position = 0;

    GC::gc();

    while (f)
    {
        PR_LOCAL_REF(f);
        GC::gc();

        // Execute ExecOp.

        if (!f->minicode_stack.empty())
        {
            MiniCode* mc = f->minicode_stack.back();
            const MiniCode::Op& op = mc->ops[mc->pos];
            mc->pos++;

            switch (op.type)
            {
            case MiniCode::Push:
                f->push(mc->objects[op.ref]);
                break;

            case MiniCode::Pop:
                f->pop();
                break;

            case MiniCode::Peek:
                {
                    ObjP p = f->stack[f->stack.size() - op.ref];
                    f->push(p);
                }
                break;

            case MiniCode::Arg:
                {
                    ObjP p = f->pop();
                    f->args.push_back(p);
                }
                break;

            case MiniCode::Call:
                call_method(symbol_to_name(mc->objects[op.ref]));
                break;

            case MiniCode::CallCall:
                call_method("call");
                break;

            case MiniCode::IfTrue:
                if (f->pop() == true_object())
                    mc->pos = op.ref;
                break;

            case MiniCode::IfFalse:
                if (f->pop() == false_object())
                    mc->pos = op.ref;
                break;

            case MiniCode::IfNull:
                if (f->pop() == 0)
                    mc->pos = op.ref;
                break;

            case MiniCode::Set:
                mc->objects[op.ref] = f->pop();
                break;

            case MiniCode::CallFunc0:
                mc->func0(mc, f);
                break;

            case MiniCode::CallFunc1:
                mc->func1(mc, f);
                break;

            case MiniCode::Goto:
                mc->pos = op.ref;
                break;

            case MiniCode::End:
                f->minicode_stack.pop_back();
                delete mc;
                break;

            case MiniCode::ListAsArgs:
                {
                    List* l = to_list(f->pop());
                    for (int i = 0; i < l->get_size(); i++)
                        f->arg(l->get(l->get_size() - i - 1));
                }
                break;

            case MiniCode::PushNull:
                f->push(0);
                break;

            case MiniCode::NextItemOrGoto:
                {
                    List *l = to_list(f->pop());
                    if (mc->counter >= l->get_size())
                        mc->pos = op.ref;
                    else
                        f->push(l->get(mc->counter++));
                }
                break;

            case MiniCode::IfCounterGE:
                if (mc->counter >= int_value(f->pop()))
                    mc->pos = op.ref;
                break;

            case MiniCode::IncCounter:
                mc->counter++;
                break;

            default: assert(0);
            }

            continue;
        }

        // Check if all the Code has been executed.

        if (f->position >= f->code->get_size())
        {
            f->in_execution = false;
            ObjP ret = f->ret;
            f = f->caller.get();
            if (f)
            {
                f->callee.set(0);
                f->push(ret);
            }
            continue;
        }

        // Execute regular Code.

        Code::Op op = f->code->get_op(f->position);
        ObjP arg = f->code->get_arg(f->position);
        PR_LOCAL_REF(arg);

        f->position++;

        switch (op)
        {
        case Code::Push:
            f->push(arg);
            break;

        case Code::Pop:
            f->pop();
            break;

        case Code::Peek:
            {
                ObjP p = f->stack[f->stack.size() - fixnum_to_int(arg)];
                f->push(p);
            }
            break;

        case Code::Method:
            {
                ObjP p = f->pop();

                Type* type = pr::get_type(p);
                assert(type);

                Name name = symbol_to_name(arg);
                Callable c = type->get_method(name);

                if (c.type == Callable::NONE)
                    handle_exception(new Exception(Name("bad_method"), arg));

                f->push(*new Method(p, c));
            }
            break;

        case Code::Arg:
            {
                ObjP p = f->pop();
                f->args.push_back(p);
            }
            break;

        case Code::CallMethod:
            call_method(symbol_to_name(arg));
            break;

        case Code::Lookup:
            {
                try
                {
                    ObjP p = f->lookup(symbol_to_name(arg));
                    f->push(p);
                }
                catch (Exception* e)
                {
                    handle_exception(e);
                }
            }
            break;

        case Code::Assign:
            {
                ObjP p = f->pop();
                f->push(p);
                f->set(symbol_to_name(arg), p);
            }
            break;

        case Code::Return:
            f->ret = f->pop();
            break;

        case Code::Closure:
            {
                Closure* closure = new Closure(f, to_code(arg));
                f->push(*closure);
            }
            break;

        case Code::List:
            {
                List* l = new List();
                PR_LOCAL_REF(l);
                int n = fixnum_to_int(arg);
                for (int i = 0; i < n; i++)
                    l->prepend(f->pop());
                f->push(*l);
            }
            break;
        }
    }
}

void Executor::call_method(Name name)
{
    Frame* old_frame = f;

    ObjP p = f->pop();

    Type* type = pr::get_type(p);
    assert(type);

    Callable c = type->get_method(name);

    ObjP ret;

    try
    {

    if (c.type == Callable::NONE)
    {
        c = type->get_method("bad_method");
        if (c.type == Callable::NONE)
            throw new Exception(Name("bad_method"), name_to_symbol(name));

        List* list = new List();
        PR_LOCAL_REF(list);
        for (int i = 0; i < (int)f->args.size(); i++)
            list->append(f->args[f->args.size() - i - 1]);
        f->args.clear();

        ret = c.call2(p, name_to_symbol(name), *list);
    }
    else
    {
        switch (f->args.size())
        {
            case 0:
                ret = c.call0(p);
                break;
            case 1:
                {
                    ObjP arg0 = f->args[0];
                    f->args.clear();
                    ret = c.call1(p, arg0);
                    break;
                }
            case 2:
                {
                    ObjP arg0 = f->args[1];
                    ObjP arg1 = f->args[0];
                    f->args.clear();
                    ret = c.call2(p, arg0, arg1);
                    break;
                }
            default:
                {
                    List* list = new List();
                    PR_LOCAL_REF(list);
                    for (int i = 0; i < (int)f->args.size(); i++)
                        list->append(f->args[f->args.size() - i - 1]);
                    f->args.clear();
                    ret = c.callx(p, list);
                }
                break;
        }
    }

    }
    catch (Exception* e)
    {
        handle_exception(e);
        return;
    }

    //assert((f->exec_op != 0) == (ret == error_object()));

    if (ret != error_object())
        old_frame->push(ret);
}

void Executor::handle_exception(Exception* e)
{
    Frame* orig_frame = f;

    Frame* ff = f;

    while (ff && ff->exc_handler == 0)
        ff = ff->caller.get();

    if (!ff)
        throw e;

    Frame* nf = new Frame(ff->previous.get(), 0, new Code());

    set_frame(nf);

    deferred_method_call2(ff->exc_handler, "call", *e, *orig_frame);
}
