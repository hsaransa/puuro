#ifndef _pr_frame_hpp_
#define _pr_frame_hpp_

#include "prdefs.hpp"
#include "object.hpp"
#include "gc.hpp"
#include "minicode.hpp"
#include <stack>
#include <vector>
#include <list>

namespace pr
{
    class Frame : public Object
    {
        friend class Executor;
        friend class Std;
    public:
        Frame(Frame* prev, Frame* caller, Code* ast);
        virtual ~Frame();

        virtual Type* get_type();
        virtual void gc_mark();

        void set_local(Name n, ObjP p);
        void set(Name n, ObjP p);
        ObjP execute();
        ObjP continue_(ObjP);

        void push(ObjP p);
        ObjP pop();
        void arg(ObjP p);

        Frame* clone_continuation();

    private:
        ObjP lookup(Name n);

        ObjP to_string_();
        ObjP set_local_(ObjP, ObjP);
        ObjP set_(ObjP, ObjP);

        ObjP pollute_(ObjP p);

        ObjP set_exception_handler(ObjP);
        ObjP get_exception_handler();

        ObjP previous_();
        ObjP caller_();
        ObjP callee_();
        ObjP code_();

        ObjP cut_previous_();

    private:
        RefCount<Frame> previous;
        RefCount<Frame> caller;
        RefCount<Frame> callee;
        RefCount<Code> code;
        int position;

        std::list<MiniCode*> minicode_stack;

        std::vector<ObjP> stack;
        std::vector<ObjP> args;

        ObjP exc_handler;

        ObjP ret;

        bool in_execution;

        std::map<Name, ObjP> locals;
    };
}

#endif
