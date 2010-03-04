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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

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
        type->add_method("socket", (Callable::mptr2)&File::socket_);
        type->add_method("connect", (Callable::mptr2)&File::connect_);
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

    file_mode = mode;

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

ObjP File::socket_(ObjP a, ObjP b)
{
    if (!is_symbol(a))
        throw new Exception("bad_type", a);
    if (!is_symbol(b))
        throw new Exception("bad_type", b);

    int domain;

    sock_domain = symbol_to_name(a);
    if (sock_domain.id() == N_unix)
        domain = AF_UNIX;
    else if (sock_domain.id() == N_inet)
        domain = AF_INET;
    else if (sock_domain.id() == N_inet6)
        domain = AF_INET6;
    else
        throw new Exception("bad_domain", a);

    int type;

    sock_type = symbol_to_name(b);
    if (sock_type.id() == N_stream)
        type = SOCK_STREAM;
    else if (sock_type.id() == N_dgram)
        type = SOCK_DGRAM;
    else
        throw new Exception("bad_type", b);

    close();
    fd = socket(domain, type, 0);
    if (fd < 0)
        throw new Exception("system_error", int_to_fixnum(errno));

    int flags;
    flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0)
        // TODO: close
        throw new Exception("system_error", int_to_fixnum(errno));

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0)
        // TODO: close
        throw new Exception("system_error", int_to_fixnum(errno));

    return 0;
}

static void connect_cb(void*, ObjP p)
{
    List* l = to_object(p)->cast_list();
    File* file = (File*)to_object(l->get(0));
    Frame* frame = (Frame*)to_object(l->get(1));

    get_executor()->set_frame(frame);

    int val;
    socklen_t len = sizeof(val);
    getsockopt(file->get_fd(), SOL_SOCKET, SO_ERROR, &val, &len);
    assert(len > 0);

    if (val != 0)
    {
        get_executor()->handle_exception(new Exception("system_error", val));
        return;
    }

    frame->push(0);
}

ObjP File::connect_(ObjP a, ObjP b)
{
    String* s = to_string(a);
    int port = int_value(b);

#if 0
    int ip0, ip1, ip2, ip3, port;
    if (sscanf(s->get_data(), "%d.%d.%d.%d:%d", &ip0, &ip1, &ip2, &ip3, &port) != 5)
        throw new Exception("bad_inetaddr", a);
#endif

    if (sock_domain.id() != N_inet)
        throw new Exception("not_implemented", 0);

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = inet_addr(s->get_data());

    int ret = ::connect(fd, (struct sockaddr*)&sin, sizeof(sin));
    if (ret < 0)
    {
        if (errno == EINPROGRESS)
        {
            Frame* f = get_executor()->get_frame();
            List* l = new List(4, (ObjP)*this, (ObjP)*f, a, b);
            get_selector()->add_watcher(fd, Selector::WRITE, connect_cb, 0, *l);
            dec_ref(l);
            get_executor()->set_frame(0);
            return error_object();
        }
    }

    throw new Exception("system_error", int_to_fixnum(errno));
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
