#include "code.hpp"
#include "ast.hpp"
#include "frame.hpp"
#include "list.hpp"
#include "primitives.hpp"
#include "gc.hpp"
#include "string.hpp"
#include "type.hpp"
#include <iostream>
#include <stdio.h>
#include <stdexcept>

using namespace pr;

Code::Code(AST* ast, bool args)
:   Object(get_type())
{
    tmp_filepos.file = Name("<none>").id();
    tmp_filepos.line = 0;

    if (args)
    {
        // Parse code parameters (pre, sink and post).

        AST* param_ast = ast->get_children()[0].get();

        int i = 0;

        for (; i < (int)param_ast->get_children().size(); i++)
        {
            if (param_ast->get_children()[i]->get_node_type().id() != N_Param)
                break;
            ObjP obj = param_ast->get_children()[i]->get_object();
            Name n = symbol_to_name(obj);
            pre_params.push_back(n);
        }

        if (i < (int)param_ast->get_children().size() &&
            param_ast->get_children()[i]->get_node_type().id() == N_ParamSink)
        {
            ObjP obj = param_ast->get_children()[i]->get_object();
            sink_param = symbol_to_name(obj);
            i++;
        }

        for (; i < (int)param_ast->get_children().size(); i++)
        {
            if (param_ast->get_children()[i]->get_node_type().id() != N_Param)
                break;
            ObjP obj = param_ast->get_children()[i]->get_object();
            Name n = symbol_to_name(obj);
            post_params.push_back(n);
        }

        compile(ast->get_children()[1].get());
    }
    else
        compile(ast);

    emit(Return);

    //debug_print();
}

Code::Code()
:   Object(get_type())
{
}

Code::~Code()
{
}

Type* Code::get_type()
{
    static Type* type;

    if (!type)
    {
        type = new Type("code");
        type->add_method("to_string", (Callable::mptr0)&Code::to_string_);
        type->add_method("pre_params", (Callable::mptr0)&Code::pre_params_);
        type->add_method("sink_param", (Callable::mptr0)&Code::sink_param_);
        type->add_method("post_params", (Callable::mptr0)&Code::post_params_);
    }

    return type;
}

void Code::gc_mark()
{
    for (int i = 0; i < (int)arguments.size(); i++)
        GC::mark(arguments[i]);
}

Code* Code::cast_code()
{
    return this;
}

void Code::emit(Op op, ObjP obj)
{
    operators.push_back(op);
    arguments.push_back(obj);
    positions.push_back(tmp_filepos);
}

void Code::compile(AST* ast)
{
    Name n = ast->get_node_type();
    AST* ch0 = ast->get_children().size() >= 1 ? ast->get_children()[0].get() : 0;
    AST* ch1 = ast->get_children().size() >= 2 ? ast->get_children()[1].get() : 0;
    //AST* ch2 = ast->get_children().size() >= 3 ? ast->get_children()[2] : 0;

    FilePosition old_pos = tmp_filepos;
    tmp_filepos = ast->get_position();

    switch (n.id())
    {
    case N_ExprList:
        if (ast->get_children().empty())
            emit(Push, 0);

        for (int i = 0; i < (int)ast->get_children().size(); i++)
        {
            compile(ast->get_children()[i].get());

            // Ignore all but last.
            if (i+1 != (int)ast->get_children().size())
                emit(Pop);
        }
        break;

    case N_Assign:
        if (ch0->get_children().size() != ch1->get_children().size())
            throw std::runtime_error("todo error message here");

        {
            int members_left = 0;

            for (int i = 0; i < (int)ch0->get_children().size(); i++)
            {
                int n = ch0->get_children()[i]->get_node_type().id();
                if (n != N_AssignMember)
                    continue;

                compile(ch0->get_children()[i]->get_children()[0].get());
                members_left++;
            }

            for (int i = 0; i < (int)ch1->get_children().size(); i++)
            {

                int n = ch0->get_children()[i]->get_node_type().id();
                if (n == N_AssignVariable)
                {
                    compile(ch1->get_children()[i].get());
                    emit(Assign, ch0->get_children()[i]->get_object());
                }
                else if (n == N_AssignMember)
                {
                    compile(ch1->get_children()[i].get());
                    emit(Push, ch0->get_children()[i]->get_object());
                    emit(Arg);
                    emit(Arg);
                    emit(Peek, int_to_fixnum(members_left));
                    members_left--;
                    emit(CallMethod, name_to_symbol("set_attribute"));
                    emit(Pop);
                }
                else
                    throw std::runtime_error("what?");
            }

#if 0
            for (int i = 0; i < (int)ch0->get_children().size(); i++)
            {
                int n = ch0->get_children()[i]->get_node_type().id();
                if (n == N_AssignMember)
                    emit(Pop);
            }
#endif
        }

        // TODO: push something?
        break;

    case N_Integer:
    case N_Symbol:
    case N_String:
        emit(Push, ast->get_object());
        break;

    case N_True:
        emit(Push, true_object());
        break;

    case N_False:
        emit(Push, false_object());
        break;

    case N_Null:
        emit(Push, 0);
        break;

    case N_Variable:
        emit(Lookup, ast->get_object());
        break;

    case N_Add:
    case N_Sub:
    case N_Mul:
    case N_Div:
    case N_Mod:
    case N_LT:
    case N_GT:
    case N_Eq:
    case N_NE:
    case N_GE:
    case N_LE:
    case N_GetItem:
        compile(ch0);
        compile(ch1);
        emit(Arg);
        {
            const char* m = 0;
            switch (n.id())
            {
            case N_Add: m = "add"; break;
            case N_Sub: m = "sub"; break;
            case N_Mul: m = "mul"; break;
            case N_Div: m = "div"; break;
            case N_Mod: m = "mod"; break;
            case N_LT: m = "lt"; break;
            case N_GT: m = "gt"; break;
            case N_Eq: m = "eq"; break;
            case N_NE: m = "ne"; break;
            case N_GE: m = "ge"; break;
            case N_LE: m = "le"; break;
            case N_GetItem: m = "at"; break;
            }
            assert(m);
            emit(CallMethod, name_to_symbol(m));
        }
        break;

    case N_Neg:
        compile(ch0);
        emit(CallMethod, name_to_symbol("neg"));
        break;

    case N_Not:
        compile(ch0);
        emit(CallMethod, name_to_symbol("not"));
        break;

    case N_Call:
        compile(ch0);
        for (int i = 0; i < (int)ch1->get_children().size(); i++)
            compile(ch1->get_children()[i].get());
        for (int i = 0; i < (int)ch1->get_children().size(); i++)
            emit(Arg);
        emit(CallMethod, name_to_symbol("call"));
        break;

    case N_CallMethod:
        compile(ch0);
        for (int i = 0; i < (int)ch1->get_children().size(); i++)
            compile(ch1->get_children()[i].get());
        for (int i = 0; i < (int)ch1->get_children().size(); i++)
            emit(Arg);
        emit(CallMethod, ast->get_object());
        break;

    case N_List:
        for (int i = 0; i < (int)ch0->get_children().size(); i++)
            compile(ch0->get_children()[i].get());
        emit(List, int_to_fixnum(ch0->get_children().size()));
        break;

    case N_Method:
        compile(ch0);
        emit(Method, ast->get_object());
        break;

    case N_Code:
        {
            Code* code = new Code(ast, true);
            emit(Closure, *code);
            dec_ref(code);
        }
        break;

    default:
        std::cerr << "TODO: emit code for " << n.s() << '\n';
        assert(0);
        break;
    }

    //tmp_filepos = old_pos;
}

void Code::debug_print()
{
    assert(operators.size() == arguments.size());

    for (int i = 0; i < (int)operators.size(); i++)
    {
        Op op = operators[i];
        ObjP arg = arguments[i];

        printf("%3d,%3d ", i, positions[i].line);

        switch (op)
        {
        case Return:     printf("return      "); break;
        case Assign:     printf("assign      "); break;
        case Push:       printf("push        "); break;
        case Pop:        printf("pop         "); break;
        case Peek:       printf("peek        "); break;
        case Lookup:     printf("lookup      "); break;
        case Method:     printf("method      "); break;
        case CallMethod: printf("callmethod  "); break;
        case Arg:        printf("arg         "); break;
        case Closure:    printf("closure     "); break;
        case List:       printf("list        "); break;
        }

        if (op != Pop && op != Arg && op != Return)
        {
            String* s = call_to_string(arg);
            printf("%s", s->get_data());
            dec_ref(s);
        }

        printf("\n");
    }
}

ObjP Code::to_string_()
{
    return *new String("<code>");
}

ObjP Code::pre_params_()
{
    pr::List* l = new pr::List();
    for (int i = 0; i < (int)pre_params.size(); i++)
        l->append(name_to_symbol(pre_params[i]));
    return *l;
}

ObjP Code::sink_param_()
{
    if (sink_param.valid())
        return name_to_symbol(sink_param);
    else
        return 0;
}

ObjP Code::post_params_()
{
    pr::List* l = new pr::List();
    for (int i = 0; i < (int)post_params.size(); i++)
        l->append(name_to_symbol(post_params[i]));
    return *l;
}
