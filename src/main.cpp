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
#include "selector.hpp"
#include "std2.hpp"
#include "resolver.hpp"
#include <stdio.h>
#include <signal.h>

using namespace pr;

static void execute_file(const char* fn, List* args)
{
    Frame* frame;

    {
        String* s = read_file(fn);
        Lexer* l = new Lexer(fn, s);
        Parser* p = new Parser(l);
        p->parse();

        AST* ast = p->get_ast();
#if 0
        ast->debug_print();
        std::cout << '\n';
#endif

        dec_ref(s);
        dec_ref(l);
        dec_ref(p);

        Code* code = new Code(ast, false);
#if 0
        code->debug_print();
        std::cout << '\n';
#endif
        dec_ref(ast);

        frame = new Frame(0, 0, code);
        dec_ref(code);

        Std* std = new Std();
        Std2* std2 = new Std2();
        Resolver* resolver = new Resolver();
        frame->set_local("std", *std);
        frame->set_local("std2", *std2);
        frame->set_local("resolver", *resolver);
        frame->set_local("args", args ? (ObjP)*args : 0);
        String* lib_dir = new String(LIB_DIR);
        frame->set_local("lib_dir", *lib_dir);
        dec_ref(lib_dir);

        dec_ref(args);
        dec_ref(std);
    }

    executor = new Executor();
    GC::add_root(executor);

    executor->set_frame(frame);
    dec_ref(frame);

    do {
        executor->execute();
        get_selector()->select();
    } while (!get_selector()->empty() || executor->get_frame());

    dec_ref(executor);

    GC::del_root(executor);
    executor = 0;
}

int main(int argc, char* argv[])
{
    // Ignore SIGPIPE.

    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &act, 0);

    // Init something.

    init_primitive_types();

    // Parse command line.

    try
    {
        List* args = new List();
        for (int i = 0; i < argc; i++)
        {
            String* s = new String(argv[i]);
            args->append(*s);
            dec_ref(s);
        }

        const char* main_file = LIB_DIR "/main.puuro";

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

#ifdef NO_GC
        std::cerr << "objects in the end: " << GC::get_object_count() << '\n';
#endif

        GC::gc();
    }
    catch (Exception* e)
    {
        std::cerr << "fatal exception\n";
        String* s = e->to_string();
        std::cerr << "exception not catched: " << s->get_data() << '\n';

        Frame* f = executor->get_frame();
        if (!f)
            std::cerr << "BUG in puuro: no current frame!\n";

        while (f)
        {
            std::cerr << "TRACE " << f->get_current_file() << " line " << f->get_current_line() << '\n';
            f = f->get_caller();
        }

        return 1;
    }

    return 0;
}
