#ifndef _pr_closure_hpp_
#define _pr_closure_hpp_

#include "prdefs.hpp"
#include "object.hpp"
#include "gc.hpp"

namespace pr
{
    class Closure : public Object
    {
    public:
        Closure(Frame* frame, Code* code);
        virtual ~Closure();

        virtual Type* get_type();
        virtual void gc_mark();

    private:
        ObjP to_string_();
        ObjP frame_();
        ObjP code_();
        ObjP call_(List*);
        ObjP call_frame_(List*);

    private:
        RefCount<Frame> frame;
        RefCount<Code> code;

        static Type* type;
    };
}

#endif
