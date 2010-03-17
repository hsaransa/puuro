#include "list.hpp"
#include "frame.hpp"

using namespace pr;

void pr::deferred_method_call0(ObjP p, Name n)
{
    static const MiniCode::Op ops[] =
    {
        { MiniCode::Push, 0 },
        { MiniCode::Call, 1 },
        { MiniCode::End, 0 },
    };

    MiniCode* mc = new MiniCode(ops);
    mc->objects.push_back(p);
    mc->objects.push_back(name_to_symbol(n));

    get_executor()->emit(mc);
}

void pr::deferred_method_call1(ObjP p, Name n, ObjP arg)
{
    static const MiniCode::Op ops[] =
    {
        { MiniCode::Push, 2 },
        { MiniCode::Arg, -1 },
        { MiniCode::Push, 0 },
        { MiniCode::Call, 1 },
        { MiniCode::End, 0 },
    };

    MiniCode* mc = new MiniCode(ops);
    mc->objects.push_back(p);
    mc->objects.push_back(name_to_symbol(n));
    mc->objects.push_back(arg);

    get_executor()->emit(mc);
}

void pr::deferred_method_call2(ObjP p, Name n, ObjP arg, ObjP arg2)
{
    static const MiniCode::Op ops[] =
    {
        { MiniCode::Push, 3 },
        { MiniCode::Arg, -1 },
        { MiniCode::Push, 2 },
        { MiniCode::Arg, -1 },
        { MiniCode::Push, 0 },
        { MiniCode::Call, 1 },
        { MiniCode::End, 0 },
    };

    MiniCode* mc = new MiniCode(ops);
    mc->objects.push_back(p);
    mc->objects.push_back(name_to_symbol(n));
    mc->objects.push_back(arg);
    mc->objects.push_back(arg2);

    get_executor()->emit(mc);
}

void pr::deferred_method_callx(ObjP p, Name n, List* l)
{
    static const MiniCode::Op ops[] =
    {
        { MiniCode::Push, 2 },
        { MiniCode::ListAsArgs, -1 },
        { MiniCode::Push, 0 },
        { MiniCode::Call, 1 },
        { MiniCode::End, 0 },
    };

    MiniCode* mc = new MiniCode(ops);
    mc->objects.push_back(p);
    mc->objects.push_back(name_to_symbol(n));
    mc->objects.push_back((ObjP)*l);

    get_executor()->emit(mc);
}
