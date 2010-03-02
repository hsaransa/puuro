#ifndef _pr_std_hpp_
#define _pr_std_hpp_

#include "prdefs.hpp"
#include "object.hpp"

namespace pr
{
    class Std : public Object
    {
    public:
        Std();
        virtual ~Std();

        virtual Type* get_type();

    private:
        ObjP print(List*);
        ObjP struct_(ObjP p);
        ObjP pollute(ObjP);
        ObjP if_(List*);
        ObjP gc();
        ObjP while_(ObjP, ObjP);
        ObjP types();
        ObjP new_type_(ObjP);
        ObjP apply_(ObjP, ObjP);
        ObjP assoc_();
        ObjP try_(ObjP, ObjP);
        ObjP caller_();
        ObjP repeat_(ObjP);
        ObjP iter_(ObjP, ObjP);
        ObjP call_with_cloned_frame(ObjP p);
        ObjP new_continuation_(ObjP p);
        ObjP raise_(ObjP);
        ObjP compile_file_(ObjP);

        static Type* type;
    };
}

#endif
