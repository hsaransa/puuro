#ifndef _pr_executor_hpp_
#define _pr_executor_hpp_

#include "prdefs.hpp"
#include "object.hpp"
#include "gc.hpp"

namespace pr
{
    class Executor : public Object
    {
    public:
        Executor();
        virtual ~Executor();

        virtual Type* get_type();
        virtual void gc_mark();

        void set_frame(Frame*);
        void call(Frame*);
        void execute();

        void emit(MiniCode* mc);

        inline Frame* get_caller_frame() { return f; }

    private:
        void call_method(Name n);
        void handle_exception(Exception*);

        Ref<Frame*> f;
    };

    extern Executor* executor;

    inline Executor* get_executor()
    {
        return executor;
    }
}

#endif
