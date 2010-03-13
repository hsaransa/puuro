#include "closure.hpp"
#include "frame.hpp"
#include "code.hpp"
#include "string.hpp"
#include "type.hpp"
#include "list.hpp"
#include "exception.hpp"
#include "primitives.hpp"
#include "gc.hpp"
#include "executor.hpp"

using namespace pr;

Type* Closure::type = 0;

Closure::Closure(Frame* frame, Code* code)
:   Object(get_type()), frame(frame), code(code)
{
//    assert(frame && code);
}

Closure::~Closure()
{
}

Type* Closure::get_type()
{
    if (!type)
    {
        type = new Type("closure");
        type->add_method("to_string", (Callable::mptr0)&Closure::to_string_);
        type->add_method("frame", (Callable::mptr0)&Closure::frame_);
        type->add_method("code", (Callable::mptr0)&Closure::code_);
        type->add_method("call", (Callable::mptrx)&Closure::call_);
        type->add_method("call_frame", (Callable::mptrx)&Closure::call_frame_);
    }
    return type;
}

void Closure::gc_mark()
{
    GC::mark(frame);
    GC::mark(code);
}

ObjP Closure::to_string_()
{
    return *new String("<closure>");
}

ObjP Closure::frame_()
{
    return inc_ref(*frame.get());
}

ObjP Closure::code_()
{
    return inc_ref(*code.get());
}

ObjP Closure::call_(List* args)
{
    Frame* f = (Frame*)to_object(call_frame_(args));

    get_executor()->call(f);

    dec_ref(f);

    return error_object();
}

ObjP Closure::call_frame_(List* args)
{
    // Check parameter count.

    const std::vector<Name>& pre = code->get_pre_params();
    Name sink = code->get_sink_param();
    const std::vector<Name>& post = code->get_post_params();
    if (sink.valid())
    {
        if (args->get_size() < (int)(pre.size() + post.size()))
            throw new Exception(Name("bad_parameter_count"), *args);
    }
    else
    {
        if (args->get_size() != (int)(pre.size() + post.size()))
            throw new Exception(Name("bad_parameter_count"), *args);
    }

    // Make frame and set local variables.

    Frame* f = new Frame(frame.get(), 0, code.get());

    int i = 0;

    for (int j = 0; j < (int)pre.size(); j++)
        f->set_local(pre[j], args->get(i++));

    if (sink.valid())
    {
        List* l = new List();
        while (i < args->get_size() - (int)post.size())
            l->append(args->get(i++));
        f->set_local(sink, *l);
        dec_ref(l);
    }

    for (int j = 0; j < (int)post.size(); j++)
        f->set_local(post[j], args->get(i++));

    return *f;
}
