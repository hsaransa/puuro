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
        virtual Frame* cast_frame();

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

        ObjP set_exception_handler_(ObjP);
        ObjP get_exception_handler_();

        ObjP previous_();
        ObjP caller_();
        ObjP callee_();
        ObjP code_();

        ObjP current_file_();
        ObjP current_line_();

        ObjP cut_previous_();

    private:
        Ref<Frame*> previous;
        Ref<Frame*> caller;
        Ref<Frame*> callee;
        Ref<Code*> code;
        int position;

        std::list<MiniCode*> minicode_stack;

        std::vector<Ref<ObjP> > stack;
        std::vector<Ref<ObjP> > args;

        Ref<ObjP> exc_handler;

        Ref<ObjP> ret;

        bool in_execution;

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
