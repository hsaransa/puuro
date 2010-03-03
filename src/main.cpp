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

        String* s = f->read_file();

        Lexer* l = new Lexer(s);

        Parser* p = new Parser(l);

        AST* ast = p->get_ast();

#if 0
        ast->debug_print();
        std::cout << '\n';
#endif

        Code* code = new Code(ast, false);

#if 0
        code->debug_print();
        std::cout << '\n';
#endif

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

        const char* main_file = "lib/main.puuro";

        for (int i = 0; i < argc; i++)
        {
            if (strcmp(argv[i], "--main") == 0)
            {
                main_file = argv[++i];
            }
        }

        if (!main_file)
            throw new Exception("missing_main", 0);

        execute_file(main_file, args);

        GC::gc();
    }
    catch (Exception* e)
    {
        PR_LOCAL_REF(e);
        String* s = e->to_string();
        std::cerr << "exception not catched: " << s->get_data() << '\n';
        return 1;
    }

    return 0;
}
