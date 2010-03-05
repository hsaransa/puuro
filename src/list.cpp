#include "list.hpp"
#include "string.hpp"
#include "type.hpp"
#include "primitives.hpp"
#include "gc.hpp"
#include "integer.hpp"
#include "minicode.hpp"
#include "executor.hpp"
#include "frame.hpp"
#include <cstdarg>

using namespace pr;

Type* List::type;

List::List()
:   Object(get_type())
{
}

List::List(int n, ...)
:   Object(get_type())
{
    va_list args;
    va_start(args, n);
    for (int i = 0; i < n; i++)
    {
        ObjP p = va_arg(args, ObjP);
        items.push_back(p);
    }
    va_end(args);
}

List::~List()
{
}

Type* List::get_type()
{
    if (!type)
    {
        type = new Type("list");
        type->add_method(Name("map"), (Callable::mptr1)&List::map_);
        type->add_method(Name("append"), (Callable::mptr1)&List::append_);
        type->add_method(Name("push"), (Callable::mptr1)&List::append_);
        type->add_method(Name("prepend"), (Callable::mptr1)&List::prepend_);
        //type->add_method(Name("filter"), (Callable::mptr1)&List::filter_);
        type->add_method(Name("each"), (Callable::mptr1)&List::each_);
        type->add_method(Name("to_string"), (Callable::mptr0)&List::to_string_);
        type->add_method(Name("pop"), (Callable::mptr0)&List::pop_);
        type->add_method(Name("empty"), (Callable::mptr0)&List::empty_);
        type->add_method(Name("size"), (Callable::mptr0)&List::size_);
        type->add_method(Name("at"), (Callable::mptr1)&List::at_);
        type->add_method(Name("set"), (Callable::mptr2)&List::set_);
        type->add_method(Name("add"), (Callable::mptr1)&List::add_);
        type->add_method(Name("first"), (Callable::mptr0)&List::first_);
        type->add_method(Name("second"), (Callable::mptr0)&List::second_);
        type->add_method(Name("all_before"), (Callable::mptr1)&List::all_before_);
        type->add_method(Name("all_after"), (Callable::mptr1)&List::all_after_);
    }

    return type;
}

void List::gc_mark()
{
    for (int i = 0; i < (int)items.size(); i++)
        GC::mark(items[i]);
}

List* List::cast_list()
{
    return this;
}

List* List::copy() const
{
    List* l = new List();
    l->items = items;
    return l;
}

ObjP List::to_string_()
{
    String* s = new String("[");
    for (int i = 0; i < (int)items.size(); i++)
    {
        if (i != 0)
            s->append(" ");
        s->append(call_to_string(items[i]));
    }
    s->append("]");
    return *s;
}

void List::append(ObjP p)
{
    items.push_back(p);
}

ObjP List::append_(ObjP p)
{
    append(p);
    return 0;
}

void List::prepend(ObjP p)
{
    items.insert(items.begin(), p);
}

ObjP List::prepend_(ObjP p)
{
    prepend(p);
    return 0;
}

static void map_continuation0(MiniCode* mc, Frame* f)
{
    List* l = to_list(mc->objects[0]);
    if (mc->counter < l->get_size())
    {
        f->push(l->get(mc->counter));
        mc->counter++;
    }
    else
        mc->pos = 6;
}

static void map_continuation1(MiniCode* mc, Frame* f)
{
    List* l = to_list(mc->objects[1]);
    l->append(f->pop());
}

ObjP List::map_(ObjP p)
{
    static const MiniCode::Op ops[] = {
        // Get next item.
        { MiniCode::CallFunc0, -1 },
        // Call procedure with the item.
        { MiniCode::Arg, -1 },
        { MiniCode::Push, 2 }, // Push procedure
        { MiniCode::Call, 3 },
        // Add item to list.
        { MiniCode::CallFunc1, -1 },
        // Repeat.
        { MiniCode::Goto, 0 },
        // End. Label 6.
        { MiniCode::Push, 1 },
        { MiniCode::End, -1 }
    };

    MiniCode* mc = new MiniCode(ops);
    mc->func0 = map_continuation0;
    mc->func1 = map_continuation1;
    mc->counter = 0;

    mc->objects.push_back((ObjP)*this);
    mc->objects.push_back((ObjP)*new List());
    mc->objects.push_back(p);
    mc->objects.push_back(name_to_symbol("call"));

    get_executor()->emit(mc);

    return error_object();
}

ObjP List::filter_(ObjP p)
{
    List* l = new List();

    for (int i = 0; i < (int)items.size(); i++)
    {
        ObjP o = method_call1(p, Name("call"), items[i]);
        if (is_true(o))
            l->append(items[i]);
    }

    return *l;
}

ObjP List::each_(ObjP p)
{
    static const MiniCode::Op ops[] = {
        PR_MC_PUSH(0)
        PR_MC_OP1(NextItemOrGoto, 6)
        PR_MC_ARG()
        PR_MC_PUSH(1)
        PR_MC_CALL_CALL()
        PR_MC_GOTO(0)
        // 6
        PR_MC_NULL()
        PR_MC_END()
    };

    MiniCode* mc = new MiniCode(ops);
    mc->objects.push_back((ObjP)*this);
    mc->objects.push_back(p);

    get_executor()->emit(mc);

    return error_object();
}

ObjP List::pop_()
{
    if (items.empty())
        throw new Exception("list_empty", *this);
    ObjP ret = items[items.size()-1];
    inc_ref(ret);
    items.pop_back();
    return ret;
}

ObjP List::empty_()
{
    return items.empty() ? true_object() : false_object();
}

ObjP List::size_()
{
    return int_to_fixnum((int)items.size());
}

ObjP List::at_(ObjP p)
{
    int i = int_value(p);
    if (i < 0 || i >= (int)items.size())
        throw new Exception("out_of_range", p);
    return inc_ref(items[i]);
}

ObjP List::set_(ObjP p, ObjP v)
{
    int i = int_value(p);
    if (i < 0 || i >= (int)items.size())
        throw new Exception("out_of_range", p);
    items[i] = v;
    return inc_ref(v);
}

ObjP List::add_(ObjP p)
{
    List* l = pr::to_list(p);

    List* l2 = copy();
    for (int i = 0; i < (int)l->items.size(); i++)
        l2->append(l->items[i]);

    return *l2;
}

ObjP List::first_()
{
    if (items.empty())
        throw new Exception("out_of_range", *this);
    return inc_ref(items[0]);
}

ObjP List::second_()
{
    if (items.size() < 2)
        throw new Exception("out_of_range", *this);
    return inc_ref(items[1]);
}

ObjP List::all_before_(ObjP p)
{
    int i = int_value(p);
    if (i < 0 || i > (int)items.size())
        throw new Exception("out_of_range", p);
    List* l2 = new List();
    for (int j = 0; j < i; j++)
        l2->append(items[j]);
    return *l2;
}

ObjP List::all_after_(ObjP p)
{
    int i = int_value(p);
    if (i < 0 || i > (int)items.size())
        throw new Exception("out_of_range", p);
    List* l2 = new List();
    for (int j = i; j < (int)items.size(); j++)
        l2->append(items[j]);
    return *l2;
}
