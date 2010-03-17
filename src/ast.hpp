#ifndef _pr_ast_hpp_
#define _pr_ast_hpp_

#include "prdefs.hpp"
#include "gc.hpp"

namespace pr
{
    struct FilePosition
    {
        FilePosition() : file(0), line(0) {}
        FilePosition(int f, int l) : file(f), line(l) {}

        int file;
        int line;
    };

    class AST : public Object
    {
    public:
        AST(Name type, const FilePosition&);
        virtual ~AST();

        virtual Type* get_type();
        virtual void gc_mark();

        // NOTE: this function steals the reference (to simplify parser code)
        void add_child(AST*);

        void set_object(ObjP p);

        void set_position(const FilePosition& p) { position = p; }

        Name get_node_type() const { return type; }
        const std::vector<Ref<AST*> >& get_children() const { return children; }
        ObjP get_object() { return object; }
        const FilePosition& get_position() { return position; }

#ifndef NDEBUG
        void debug_print(int in=0);
#endif

    private:
        Name type;
        std::vector<Ref<AST*> > children;
        Ref<ObjP> object;
        FilePosition position;
    };
}

#endif
