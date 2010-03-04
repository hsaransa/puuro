#include "file.hpp"
#include "exception.hpp"
#include "string.hpp"
#include "type.hpp"
#include "list.hpp"
#include "selector.hpp"
#include "executor.hpp"
#include "frame.hpp"
#include "integer.hpp"
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace pr;

File::File()
:   Object(get_type())
{
    fd = -1;
}

File::~File()
{
    close();
}

Type* File::get_type()
{
    static Type* type;
    if (!type)
    {
        type = new Type(Name("file"));
        type->add_method("set_filename", (Callable::mptr1)&File::set_filename_);
        type->add_method("open", (Callable::mptr1)&File::open_);
        type->add_method("read", (Callable::mptr1)&File::read_);
    }
    return type;
}

void File::set_filename(const char* fn)
{
    filename = fn;
}

void File::open(Name mode)
{
    if (filename.empty())
        throw new Exception("no_filename", *this);

    close();
    fd = ::open(filename.c_str(), O_RDONLY);
    if (fd < 0)
        throw new Exception("system_error", int_to_fixnum(errno));
}

void File::close()
{
    if (fd >= 0)
        ::close(fd);
    fd = -1;
}

ObjP File::set_filename_(ObjP p)
{
    String* s = to_string(p);
    set_filename(s->get_data());
    return 0;
}

ObjP File::open_(ObjP p)
{
    if (!is_symbol(p))
        throw new Exception("bad_type", p);
    open(symbol_to_name(p));
    return 0;
}

static void read_cb(void* user, ObjP p)
{
    List* l = to_object(p)->cast_list();
    File* file = (File*)to_object(l->get(0));
    Frame* frame = (Frame*)to_object(l->get(1));
    int n = (int)user;

    char* buf = new char [n];
    int ret = read(file->get_fd(), buf, n);
    assert(ret >= 0);

    frame->push(*new String(buf, ret));
    delete [] buf;

    get_executor()->set_frame(frame);
}

ObjP File::read_(ObjP p)
{
    Frame* f = get_executor()->get_frame();
    int n = int_value(p);

    List* l = new List(3, (ObjP)*this, (ObjP)*f, p);
    get_selector()->add_watcher(fd, Selector::READ, read_cb, (void*)n, *l);
    dec_ref(l);

    get_executor()->set_frame(0);

    return error_object();
}
