#ifndef _pr_scope_hpp_
#define _pr_scope_hpp_

#include "prdefs.hpp"
#include "object.hpp"
#include "gc.hpp"

namespace pr
{
    class Scope : public Object
    {
    public:
        Scope(Scope* parent);
        virtual ~Scope();

        virtual Type* get_type();
        virtual void gc_mark();

        void set_local(Name n, ObjP p);
        void set(Name n, ObjP p);
        ObjP lookup(Name n);

    private:
        ObjP set_local_(ObjP, ObjP);
        ObjP set_(ObjP, ObjP);
        ObjP pollute_(ObjP);

        Ref<Scope*> parent;
        std::map<Name, Ref<ObjP> > locals;
    };
}

#endif
