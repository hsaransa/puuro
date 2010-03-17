#ifndef _pr_type_hpp_
#define _pr_type_hpp_

#include "prdefs.hpp"
#include "callable.hpp"
#include "string.hpp"

namespace pr
{
    class Type : public Object
    {
        friend class Std;
        friend class Frame; // TODO: friends could be avoided
        friend class Scope;
    public:
        Type(Name n);
        virtual ~Type();

        virtual Type* get_type();
        virtual void gc_mark();

        void add_method(Name n, Callable c);
        Callable get_method(Name n);

        inline Name get_name() const { return name; }

    private:
        ObjP name_();
        ObjP to_string_();
        ObjP get_method_(ObjP);
        ObjP set_method_(ObjP, ObjP);
        ObjP methods_();

    private:
        Name name;
        std::map<Name, Callable> methods;
        // TODO: store Callable for call separately to speed it up

        static Type* type;
    };

#if 1
    inline ObjP method_call0(ObjP p, Name n)
    {
        return get_type(p)->get_method(n).call0(p);
    }

    inline ObjP method_call1(ObjP p, Name n, ObjP arg)
    {
        return get_type(p)->get_method(n).call1(p, arg);
    }

    inline ObjP method_call2(ObjP p, Name n, ObjP arg, ObjP arg2)
    {
        return get_type(p)->get_method(n).call2(p, arg, arg2);
    }

    inline ObjP method_callx(ObjP p, Name n, List* l)
    {
        return get_type(p)->get_method(n).callx(p, l);
    }
#endif

    inline String* call_to_string(ObjP p)
    {
        ObjP ret = method_call0(p, Name("to_string"));
        return to_string(ret);
    }
}

#endif
