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
        type->add_method("write", (Callable::mptr1)&File::write_);
        type->add_method("close", (Callable::mptr0)&File::close_);
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

    int flags;

    if (mode.id() == N_r)
        flags = O_RDONLY;
    else if (mode.id() == N_w)
        flags = O_WRONLY | O_TRUNC | O_CREAT;
    else if (mode.id() == N_rw)
        flags = O_RDWR;
    else
        throw new Exception("bad_mode", name_to_symbol(mode));

    mode = mode;

    close();
    fd = ::open(filename.c_str(), flags, 0666);
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
    int n = *(int*)&user;

    get_executor()->set_frame(frame);

    char* buf = new char [n];
    int ret = read(file->get_fd(), buf, n);
    if (ret < 0)
    {
        get_executor()->handle_exception(new Exception("system_error", int_to_fixnum(errno)));
        return;
    }
    if (ret == 0)
    {
        frame->push(0);
        return;
    }

    frame->push(*new String(buf, ret));
    delete [] buf;
}

ObjP File::read_(ObjP p)
{
    Frame* f = get_executor()->get_frame();
    int n = int_value(p);

    List* l = new List(2, (ObjP)*this, (ObjP)*f);
    get_selector()->add_watcher(fd, Selector::READ, read_cb, (void*)n, *l);
    dec_ref(l);

    get_executor()->set_frame(0);

    return error_object();
}

static void write_cb(void*, ObjP p)
{
    List* l = to_object(p)->cast_list();
    File* file = (File*)to_object(l->get(0));
    Frame* frame = (Frame*)to_object(l->get(1));
    String* data = (String*)to_object(l->get(2));

    get_executor()->set_frame(frame);

    int ret = write(file->get_fd(), data->get_data(), data->get_size());
    if (ret < 0)
    {
        get_executor()->handle_exception(new Exception("system_error", int_to_fixnum(errno)));
        return;
    }

    frame->push(int_to_fixnum(ret));
}

ObjP File::write_(ObjP p)
{
    String* str = to_string(p);
    Frame* f = get_executor()->get_frame();

    List* l = new List(3, (ObjP)*this, (ObjP)*f, (ObjP)*str);
    get_selector()->add_watcher(fd, Selector::WRITE, write_cb, 0, *l);
    dec_ref(l);

    get_executor()->set_frame(0);

    return error_object();
}

ObjP File::close_()
{
    close();
    return 0;
}
