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
        enum State
        {
            READY,
            IN_EXECUTION,
            IN_CALL,
            FINISHED
        };

        Frame(Frame* prev, Frame* caller, Code* ast);
        virtual ~Frame();

        virtual Type* get_type();
        virtual void gc_mark();
        virtual Frame* cast_frame();

        void set_local(Name n, ObjP p);
        void set(Name n, ObjP p);
        ObjP execute();
        ObjP continue_(ObjP);

        void push(ObjP p);
        ObjP pop();
        void arg(ObjP p);

        Frame* clone_continuation();

        const char* get_current_file();
        int get_current_line();

        ObjP set_exception_handler_(ObjP);
        ObjP get_exception_handler_();

        Frame* get_caller() { return caller.get(); }

    private:
        ObjP lookup(Name n);

        ObjP to_string_();
        ObjP set_local_(ObjP, ObjP);
        ObjP set_(ObjP, ObjP);

        ObjP pollute_(ObjP p);

        ObjP previous_();
        ObjP caller_();
        ObjP code_();
        ObjP state_();

        ObjP current_file_();
        ObjP current_line_();

        ObjP cut_previous_();

    private:
        Ref<Frame*> previous;
        Ref<Frame*> caller;
        Ref<Code*> code;
        State state;
        int position;

        std::list<MiniCode*> minicode_stack;

        std::vector<Ref<ObjP> > stack;
        std::vector<Ref<ObjP> > args;

        Ref<ObjP> exc_handler;

        Ref<ObjP> ret;

        std::map<Name, Ref<ObjP> > locals;
    };

    inline Frame* to_frame(ObjP p)
    {
        if (is_object(p))
            return to_object(p)->cast_frame();
        throw new Exception("bad_type", p);
    }
}

#endif
