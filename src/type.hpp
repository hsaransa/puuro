#ifndef _pr_type_hpp_
#define _pr_type_hpp_

#include "prdefs.hpp"
#include "object.hpp"
#include "nametable.hpp"
#include "callable.hpp"
#include <string>
#include <set>
#include <map>

namespace pr
{
    class Type : public Object
    {
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

        friend class Std;

        static Type* type;
    };
}

#endif
