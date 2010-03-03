#ifndef _pr_callable_hpp_
#define _pr_callable_hpp_

#include "prdefs.hpp"
#include "gc.hpp"

namespace pr
{
    struct Callable
    {
        typedef ObjP (*fptr0)(ObjP);
        typedef ObjP (*fptr1)(ObjP, ObjP);
        typedef ObjP (*fptr2)(ObjP, ObjP, ObjP);
        typedef ObjP (*fptrx)(ObjP, List*);
        typedef ObjP (Object::*mptr0)();
        typedef ObjP (Object::*mptr1)(ObjP);
        typedef ObjP (Object::*mptr2)(ObjP, ObjP);
        typedef ObjP (Object::*mptrx)(List*);

        enum CallType
        {
            NONE = 0,
            FUNCTION0,
            FUNCTION1,
            FUNCTION2,
            FUNCTIONX,
            METHOD0,
            METHOD1,
            METHOD2,
            METHODX,
            OBJECT,
        };

        Callable() : type(NONE) {}
        Callable(fptr0 f) : type(FUNCTION0), func0(f) { }
        Callable(fptr1 f) : type(FUNCTION1), func1(f) { }
        Callable(fptr2 f) : type(FUNCTION2), func2(f) { }
        Callable(fptrx f) : type(FUNCTIONX), funcx(f) { }
        Callable(mptr0 m) : type(METHOD0), method0(m) { }
        Callable(mptr1 m) : type(METHOD1), method1(m) { }
        Callable(mptr2 m) : type(METHOD2), method2(m) { }
        Callable(mptrx m) : type(METHODX), methodx(m) { }
        Callable(ObjP o) : type(OBJECT), obj(o) { inc_ref(obj); }
        Callable(const Callable& c)
        {
            memcpy(this, &c, sizeof(*this));
            if (type == OBJECT)
                inc_ref(obj);
        }

        ~Callable() { if (type == OBJECT) dec_ref(obj); }

        ObjP callx(ObjP obj, List*);
        ObjP call0(ObjP obj);
        ObjP call1(ObjP obj, ObjP arg);
        ObjP call2(ObjP obj, ObjP arg, ObjP arg2);

        void gc_mark()
        {
            if (type == OBJECT)
                GC::mark(obj);
        }

        CallType type;

    private:
        union
        {
            fptr0 func0;
            fptr1 func1;
            fptr2 func2;
            fptrx funcx;
            mptr0 method0;
            mptr1 method1;
            mptr2 method2;
            mptrx methodx;
            ObjP obj;
        };

    private:
        void operator=(const Callable&) {}
    };
}

#endif
