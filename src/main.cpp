#include <iostream>
#include "file.hpp"
#include "string.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "ast.hpp"
#include "exception.hpp"
#include "code.hpp"
#include "primitives.hpp"
#include "frame.hpp"
#include "std.hpp"
#include "gc.hpp"
#include "executor.hpp"
#include "list.hpp"

using namespace pr;

static void execute_file(const char* fn, List* args)
{
            Frame* frame;

            {
                File* f = new File(fn);
                PR_LOCAL_REF(f);

                String* s = f->read_file();
                PR_LOCAL_REF(s);

                Lexer* l = new Lexer(s);
                PR_LOCAL_REF(l);

                Parser* p = new Parser(l);
                PR_LOCAL_REF(p);

                AST* ast = p->get_ast();
                PR_LOCAL_REF(ast);

                ast->debug_print();
                std::cout << '\n';

                Code* code = new Code(ast, false);
                PR_LOCAL_REF(code);
                code->debug_print();
                std::cout << '\n';

                frame = new Frame(0, 0, code);
                GC::add_root(frame);

                frame->set_local("std", *new Std());
                frame->set_local("args", args ? (ObjP)*args : 0);
            }

            {
                executor = new Executor();
                PR_LOCAL_REF(executor);
                executor->set_frame(frame);
                executor->execute();
                executor = 0;
            }

            GC::del_root(frame);
}

int main(int argc, char* argv[])
{
    init_primitive_types();

    // Parse command line.

    try
    {
        List* args = new List();
        for (int i = 0; i < argc; i++)
            args->append(*new String(argv[i]));

        execute_file("lib/main.puuro", args);

        GC::gc();
    }
    catch (Exception* e)
    {
        PR_LOCAL_REF(e);
        String* s = e->to_string();
        std::cerr << "exception: " << s->get_data() << '\n';
    }

    return 0;
}
