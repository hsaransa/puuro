#include "sockaddr.hpp"
#include "type.hpp"
#include "integer.hpp"
#include <string.h>
#include <stdio.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

using namespace pr;

SockAddr::SockAddr(int family, void* a)
:   Object(get_type())
{
    memset(&addr_un, 0, sizeof(sockaddr));
    addr.sa_family = family;
    switch (family)
    {
    case AF_INET:
        memcpy(&addr_in.sin_addr, a, sizeof(addr_in.sin_addr));
        break;

    case AF_INET6:
        memcpy(&addr_in6.sin6_addr, a, sizeof(addr_in6.sin6_addr));
        break;
//    case AF_UNIX: memcpy(&addr, a, sizeof(struct sockaddr_un)); break;
    }
}

SockAddr::SockAddr(void* a, int size)
:   Object(get_type())
{
    memset(&addr_un, 0, sizeof(sockaddr));
    assert(size <= (int)sizeof(addr_un));
    memcpy(&addr_un, a, size);
}

SockAddr::~SockAddr()
{
}

Type* SockAddr::get_type()
{
    static Type* type;
    if (!type)
    {
        type = new Type("sockaddr");
        type->add_method("to_string", (Callable::mptr0)&SockAddr::to_string_);
        type->add_method("family", (Callable::mptr0)&SockAddr::family_);
        type->add_method("set_addr", (Callable::mptr1)&SockAddr::set_addr_);
        type->add_method("set_port", (Callable::mptr1)&SockAddr::set_port_);
    }
    return type;
}

ObjP SockAddr::to_string_()
{
    switch (addr.sa_family)
    {
    case AF_INET:
        return *new String(inet_ntoa(addr_in.sin_addr));

    //case AF_INET6:
    //    return *new String(inet6_ntoa(((struct sockaddr_in6*)&addr)->sin6_addr));

    default:
        return *new String("<sockaddr, unknown family>");
    }
}

ObjP SockAddr::family_()
{
    switch (addr.sa_family)
    {
    case AF_INET: return name_to_symbol("AF_INET"); break;
    case AF_INET6: return name_to_symbol("AF_INET6"); break;
    case AF_UNIX: return name_to_symbol("AF_UNIX"); break;
    }
    return 0;
}

ObjP SockAddr::set_addr_(ObjP p)
{
    String* s = to_string(p);
    (void)s;
    assert(0);
    return 0;
}

ObjP SockAddr::set_port_(ObjP p)
{
    int port = int_value(p);
    switch (addr.sa_family)
    {
    case AF_INET:
        addr_in.sin_port = htons(port);
        break;

    case AF_INET6:
        addr_in6.sin6_port = htons(port);
        break;
    }
    return 0;
}

struct sockaddr* SockAddr::get_sockaddr()
{
    return &addr;
}

int SockAddr::get_sockaddr_size()
{
    switch (addr.sa_family)
    {
    case AF_INET: return sizeof(addr_in);
    case AF_INET6: return sizeof(addr_in6);
    case AF_UNIX: return sizeof(addr_un);
    }
    return 0;
}
