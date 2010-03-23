#ifndef _pr_frame_hpp_
#define _pr_frame_hpp_

#include "prdefs.hpp"
#include "minicode.hpp"
#include "exception.hpp"
#include <list>

namespace pr
{
    class Frame : public Object
    {
        friend class Executor;
        friend class BuiltIn;
    public:
        enum State
        {
            READY,
            IN_EXECUTION,
            IN_CALL,
            FINISHED
        };

        Frame(Scope* scope, Frame* caller, Code* ast);
        virtual ~Frame();

        virtual Type* get_type();
        virtual void gc_mark();
        virtual Frame* cast_frame();

        ObjP execute();
        ObjP continue_(ObjP);
        ObjP switch_(ObjP);

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
        ObjP to_string_();

        ObjP pollute_(ObjP p);

        ObjP scope_();
        ObjP caller_();
        ObjP code_();
        ObjP state_();

        ObjP current_file_();
        ObjP current_line_();

    private:
        Ref<Scope*> scope;
        Ref<Frame*> caller;
        Ref<Code*> code;
        State state;
        int position;

        std::list<MiniCode*> minicode_stack;

        std::vector<Ref<ObjP> > stack;
        std::vector<Ref<ObjP> > args;

        Ref<ObjP> exc_handler;

        Ref<ObjP> ret;
    };

    inline Frame* to_frame(ObjP p)
    {
        if (is_object(p))
            return to_object(p)->cast_frame();
        throw new Exception("bad_type", p);
    }
}

#endif
