#include "executor.hpp"
#include "frame.hpp"
#include "code.hpp"
#include "method.hpp"
#include "closure.hpp"
#include "list.hpp"
#include "minicode.hpp"
#include "integer.hpp"
#include "scope.hpp"
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
    GC::mark(f);
}

void Executor::emit(MiniCode* mc)
{
    assert(mc);
    f->minicode_stack.push_back(mc);
}

void Executor::set_frame(Frame* nf)
{
    assert(!nf || (nf->state == Frame::READY || nf->state == Frame::IN_EXECUTION));

    f = nf;

    if (f && f->state == Frame::READY)
        f->state = Frame::IN_EXECUTION;
}

void Executor::call(Frame* nf)
{
    assert(nf);
    assert(nf->state == Frame::READY);
    assert(f);
    assert(f->state == Frame::IN_EXECUTION);

    f->state = Frame::IN_CALL;
    nf->caller = f;

    set_frame(nf);
}

void Executor::execute()
{
    if (f->state == Frame::READY)
        f->state = Frame::IN_EXECUTION;

    while (f.get())
    {
        assert(f->state == Frame::IN_EXECUTION);

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
                dec_ref(f->pop());
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
                    dec_ref(p);
                }
                break;

            case MiniCode::Call:
                call_method(symbol_to_name(mc->objects[op.ref]));
                break;

            case MiniCode::CallCall:
                call_method("call");
                break;

            case MiniCode::IfTrue:
                {
                    ObjP p = f->pop();
                    if (p == true_object())
                        mc->pos = op.ref;
                    dec_ref(p);
                }
                break;

            case MiniCode::IfFalse:
                {
                    ObjP p = f->pop();
                    if (p == false_object())
                        mc->pos = op.ref;
                    dec_ref(p);
                }
                break;

            case MiniCode::IfNull:
                {
                    ObjP p = f->pop();
                    if (p == 0)
                        mc->pos = op.ref;
                    dec_ref(p);
                }
                break;

            case MiniCode::Set:
                mc->objects[op.ref] = f->pop();
                dec_ref(mc->objects[op.ref]);
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
                    ObjP p = f->pop();
                    List* l = to_list(p);
                    for (int i = 0; i < l->get_size(); i++)
                        f->arg(l->get(l->get_size() - i - 1));
                    dec_ref(p);
                }
                break;

            case MiniCode::PushNull:
                f->push(0);
                break;

            case MiniCode::NextItemOrGoto:
                {
                    ObjP p = f->pop();
                    List *l = to_list(p);
                    if (mc->counter >= l->get_size())
                        mc->pos = op.ref;
                    else
                        f->push(l->get(mc->counter++));
                    dec_ref(p);
                }
                break;

            case MiniCode::IfCounterGE:
                {
                    ObjP p = f->pop();
                    if (mc->counter >= int_value(p))
                        mc->pos = op.ref;
                    dec_ref(p);
                }
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
            // TODO: clean up, it's messy due to intensive debugging
            Frame* old = f;

            f->state = Frame::FINISHED;
            Ref<ObjP> ret = f->ret;

            assert(!f->caller || f->caller->state == Frame::IN_CALL);

            Ref<Frame*> nf = f->caller.get();
            old->caller.set(0);

            f = nf;

            if (f)
            {
                f->state = Frame::IN_EXECUTION;
                f->push(ret);
            }

            continue;
        }

        // Execute regular Code.

        Code::Op op = f->code->get_op(f->position);
        ObjP arg = f->code->get_arg(f->position);

        f->position++;

        switch (op)
        {
        case Code::Push:
            f->push(arg);
            break;

        case Code::Pop:
            dec_ref(f->pop());
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

                Method* m = new Method(p, c);
                f->push(*m);

                dec_ref(m);
                dec_ref(p);
            }
            break;

        case Code::Arg:
            {
                ObjP p = f->pop();
                f->args.push_back(p);
                dec_ref(p);
            }
            break;

        case Code::CallMethod:
            call_method(symbol_to_name(arg));
            break;

        case Code::Lookup:
            {
                try
                {
                    ObjP p = f->scope->lookup(symbol_to_name(arg));
                    f->push(p);
                    dec_ref(p);
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
                f->scope->set(symbol_to_name(arg), p);
                dec_ref(p);
            }
            break;

        case Code::Return:
            {
                ObjP p = f->pop();
                f->ret = p;
                dec_ref(p);
            }
            break;

        case Code::Closure:
            {
                Closure* closure = new Closure(f->scope, to_code(arg));
                f->push(*closure);
                dec_ref(closure);
            }
            break;

        case Code::List:
            {
                List* l = new List();
                int n = fixnum_to_int(arg);
                for (int i = 0; i < n; i++)
                {
                    ObjP p = f->pop();
                    l->prepend(p);
                    dec_ref(p);
                }
                f->push(*l);
                dec_ref(l);
            }
            break;

        case Code::CopyList:
            try {
                ObjP p = f->pop();
                List* l = to_list(p);
                f->push(*l->copy());
                dec_ref(p);
            } catch (Exception* e)
            {
                handle_exception(e);
            };
            break;

        case Code::ExtractFirst:
            try {
                ObjP p = f->pop();
                List* l = to_list(p);
                ObjP p2 = l->pop_first_();
                f->push(p);
                f->push(p2);
                dec_ref(p2);
                dec_ref(p);
            } catch (Exception* e)
            {
                handle_exception(e);
            };
            break;

        case Code::ExtractSink:
            try {
                ObjP p = f->pop();
                List* l = to_list(p);
                int n = l->get_size() - int_value(arg);
                ObjP p2 = l->all_after_(int_to_fixnum(n));
                f->push(p2);
                ObjP p3 = l->all_before_(int_to_fixnum(n));
                f->push(p3);
                dec_ref(p3);
                dec_ref(p2);
                dec_ref(p);
            } catch (Exception* e)
            {
                handle_exception(e);
            };
            break;

        case Code::PopEmptyList:
            try {
                ObjP p = f->pop();
                List* l = to_list(p);
                if (l->get_size())
                    throw new Exception("too_many_items", 0);
                dec_ref(p);
            } catch (Exception* e)
            {
                handle_exception(e);
            };
            break;
        }
    }
}

void Executor::call_method(Name name)
{
    Frame* old_frame = f;

    ObjP obj = f->pop();
    Type* type = pr::get_type(obj);
    assert(type);

    ObjP ret;

    try
    {
        Callable c = type->get_method(name);

        if (c.type == Callable::NONE)
        {
            Callable c = type->get_method("bad_method");
            if (c.type == Callable::NONE)
                throw new Exception(Name("bad_method"), name_to_symbol(name));

            List* list = new List();
            for (int i = 0; i < (int)f->args.size(); i++)
                list->append(f->args[f->args.size() - i - 1]);
            f->args.clear();

            ret = c.call2(obj, name_to_symbol(name), *list);
            dec_ref(list);
        }
        else
        {
            switch (f->args.size())
            {
                case 0:
                    ret = c.call0(obj);
                    break;
                case 1:
                    {
                        ObjP arg0 = inc_ref(f->args[0]);
                        f->args.clear();
                        ret = c.call1(obj, arg0);
                        dec_ref(arg0);
                        break;
                    }
                    break;
                case 2:
                    {
                        ObjP arg0 = inc_ref(f->args[1]);
                        ObjP arg1 = inc_ref(f->args[0]);
                        f->args.clear();
                        ret = c.call2(obj, arg0, arg1);
                        dec_ref(arg0);
                        dec_ref(arg1);
                        break;
                    }
                default:
                    {
                        List* list = new List();
                        for (int i = 0; i < (int)f->args.size(); i++)
                            list->append(f->args[f->args.size() - i - 1]);
                        f->args.clear();
                        ret = c.callx(obj, list);
                        dec_ref(list);
                    }
                    break;
            }

            dec_ref(obj);
        }
    }
    catch (Exception* e)
    {
        handle_exception(e);
        return;
    }

    // If error was returned, frame must have changed or there's new minicode.
    // TODO: check that minicode was emitted, this test isn't enough
    assert(ret != error_object() || (f != old_frame || !f->minicode_stack.empty()));

    if (ret != error_object())
    {
        old_frame->push(ret);
        dec_ref(ret);
    }
}

void Executor::handle_exception(Exception* e)
{
    Ref<Frame*> orig_frame = f;

    Frame* ff = f;

    while (ff && ff->exc_handler == 0)
        ff = ff->caller.get();

    if (!ff)
        throw e;

    Frame* nf = new Frame(ff->scope.get(), 0, new Code());

    set_frame(nf);

    deferred_method_call2(ff->exc_handler, "call", *e, *orig_frame.get());
}
