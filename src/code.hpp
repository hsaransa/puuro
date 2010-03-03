#ifndef _pr_code_hpp_
#define _pr_code_hpp_

#include "prdefs.hpp"
#include "object.hpp"
#include "nametable.hpp"
#include "primitives.hpp"
#include "gc.hpp"
#include "ast.hpp"
#include <vector>

namespace pr
{
    class Code : public Object
    {
    public:
        enum Op
        {
            Return,
            Assign,
            Push,
            Pop,
            Peek,
            Lookup,
            Arg,
            CallMethod,
            Method,
            Closure,
            List,
        };

        Code(AST*, bool args);
        Code();
        virtual ~Code();

        virtual Type* get_type();
        virtual void gc_mark();
        virtual Code* cast_code();

        const std::vector<Name>& get_pre_params() const { return pre_params; }
        const Name get_sink_param() const { return sink_param; }
        const std::vector<Name>& get_post_params() const { return post_params; }

        int get_size() const { return (int)operators.size(); }
        Op get_op(int i) const { assert(i >= 0 && i < get_size()); return operators[i]; }
        ObjP get_arg(int i) const { assert(i >= 0 && i < get_size()); return arguments[i]; }
        const FilePosition& get_position(int i) const { assert(i >= 0 && i < get_size()); return positions[i]; }

        void debug_print();

    private:
        ObjP to_string_();
        ObjP pre_params_();
        ObjP sink_param_();
        ObjP post_params_();

        void compile(AST* ast);
        void emit(Op op, ObjP obj=0);

        std::vector<Name> pre_params;
        Name sink_param;
        std::vector<Name> post_params;
        std::vector<Op> operators;
        std::vector<Ref<ObjP> > arguments;
        std::vector<FilePosition> positions;

        FilePosition tmp_filepos;
    };

    inline Code* to_code(ObjP p)
    {
        if (is_object(p))
            return to_object(p)->cast_code();
        throw new Exception("bad_type", p);
    }
}

#endif
