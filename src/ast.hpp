#ifndef _pr_ast_hpp_
#define _pr_ast_hpp_

#include "prdefs.hpp"
#include "object.hpp"
#include "nametable.hpp"
#include "gc.hpp"
#include <vector>

namespace pr
{
    class AST : public Object
    {
    public:
        AST(Name type);
        virtual ~AST();

        virtual Type* get_type();
        virtual void gc_mark();

        void add_child(AST*);
        void set_object(ObjP p);

        Name get_node_type() { return type; }
        const std::vector<AST*>& get_children() const { return children; }
        ObjP get_object() { return object; }

#ifndef NDEBUG
        void debug_print(int in=0);
#endif

    private:
        Name type;
        std::vector<AST*> children;
        ObjP object;
    };
}

#endif
