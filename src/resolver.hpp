#ifndef _pr_resolver_hpp_
#define _pr_resolver_hpp_

#include "prdefs.hpp"
#include "object.hpp"

namespace pr
{
    class Resolver : public Object
    {
    public:
        Resolver();
        virtual ~Resolver();

        virtual Type* get_type();

    private:
        ObjP resolve_(ObjP);
    };
}

#endif
