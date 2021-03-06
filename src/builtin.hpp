#ifndef _pr_builtin_hpp_
#define _pr_builtin_hpp_

#include "prdefs.hpp"
#include "object.hpp"

namespace pr
{
    class BuiltIn : public Object
    {
    public:
        BuiltIn();
        virtual ~BuiltIn();

        virtual Type* get_type();

    private:
        ObjP print(List*);
        ObjP pollute(ObjP);
        ObjP if_(List*);
        ObjP gc();
        ObjP gc_obj_count_();
        ObjP while_(ObjP, ObjP);
        ObjP new_type_(ObjP);
        ObjP apply_(ObjP, ObjP);
        ObjP assoc_();
        ObjP caller_();
        ObjP repeat_(ObjP);
        ObjP iter_(ObjP, ObjP);
        ObjP call_with_cloned_frame(ObjP p);
        ObjP new_continuation_(ObjP p);
        ObjP compile_file_(ObjP);
        ObjP exception_(ObjP, ObjP);
        ObjP sleep_(ObjP, ObjP);
        ObjP file_();
        ObjP compile_string_(ObjP, ObjP);
        ObjP sockaddr_(ObjP);
        ObjP scope_(ObjP);

        static Type* type;
    };
}

#endif
