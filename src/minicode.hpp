#ifndef _pr_minicode_hpp_
#define _pr_minicode_hpp_

#include "prdefs.hpp"
#include "gc.hpp"
#include "executor.hpp"
#include <set>
#include <stack>

namespace pr
{
    class MiniCode
    {
    public:
        enum Type
        {
            Push,
            Pop,
            Peek,
            Arg,
            Call,
            IfTrue,
            IfFalse,
            IfNull,
            Set,
            CallFunc0,
            CallFunc1,
            Goto,
            End,
            ListAsArgs,
            PushNull,
            NextItemOrGoto,
            CallCall,
            IfCounterGE,
            IncCounter,
        };

        struct Op
        {
            Type type;
            int ref;
        };

        MiniCode(const Op* ops)
        :   ops(ops), pos(0), func0(0), func1(0), counter(0)
        {
        }

        void gc_mark()
        {
            for (int i = 0; i < (int)objects.size(); i++)
                GC::mark(objects[i]);
        }

        MiniCode* clone()
        {
            MiniCode* mc = new MiniCode(ops);
            mc->objects  = objects;
            mc->pos      = pos;
            mc->func0    = func0;
            mc->func1    = func1;
            mc->counter  = counter;
            return mc;
        }

    public:
        const Op* ops;
        std::vector<Ref<ObjP> > objects;
        int pos;
        void (*func0)(MiniCode*, Frame*);
        void (*func1)(MiniCode*, Frame*);
        int counter;

    private:
        MiniCode(const MiniCode&) {}
        void operator=(const MiniCode&) {}
    };

#define PR_MC_OP0(op)    { MiniCode::op, -1 },
#define PR_MC_OP1(op, a) { MiniCode::op, a },

#define PR_MC_PUSH(x)     PR_MC_OP1(Push, x)
#define PR_MC_CALL(x)     PR_MC_OP1(Call, x)
#define PR_MC_GOTO(x)     PR_MC_OP1(Goto, x)
#define PR_MC_IF_FALSE(x) PR_MC_OP1(IfFalse, x)
#define PR_MC_SET(x)      PR_MC_OP1(Set, x)
#define PR_MC_FUNC0()     PR_MC_OP0(CallFunc0)
#define PR_MC_FUNC1()     PR_MC_OP0(CallFunc1)
#define PR_MC_END()       PR_MC_OP0(End)
#define PR_MC_NULL()      PR_MC_OP0(PushNull)
#define PR_MC_ARG()      PR_MC_OP0(Arg)
#define PR_MC_CALL_CALL() PR_MC_OP0(CallCall)

    void deferred_method_call0(ObjP p, Name n);
    void deferred_method_call1(ObjP p, Name n, ObjP);
    void deferred_method_call2(ObjP p, Name n, ObjP, ObjP);
    void deferred_method_callx(ObjP p, Name n, List* l);
};

#endif
