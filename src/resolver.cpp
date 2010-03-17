#include "resolver.hpp"
#include "type.hpp"
#include "list.hpp"
#include "sockaddr.hpp"
#include <netdb.h>

using namespace pr;

Resolver::Resolver()
:   Object(get_type())
{
}

Resolver::~Resolver()
{
}

Type* Resolver::get_type()
{
    static Type* type;
    if (!type)
    {
        type = new Type("resolver");
        type->add_method("resolve", (Callable::mptr1)&Resolver::resolve_);
        type->add_method("inaddr_any", (Callable::mptr0)&Resolver::inaddr_any_);
        type->add_method("inaddr_none", (Callable::mptr0)&Resolver::inaddr_none_);
    }
    return type;
}

ObjP Resolver::resolve_(ObjP p)
{
    String* s = to_string(p);
    struct hostent* he = gethostbyname(s->get_data());

    List* l = new List();

    char** h = he->h_addr_list;
    while (*h)
    {
        l->append(*new SockAddr(he->h_addrtype, (void*)*h));
        h++;
    }

    return *l;
}

ObjP Resolver::inaddr_any_()
{
    struct in_addr addr;
    addr.s_addr = INADDR_ANY;
    return *new SockAddr(AF_INET, &addr);
}

ObjP Resolver::inaddr_none_()
{
    struct in_addr addr;
    addr.s_addr = INADDR_NONE;
    return *new SockAddr(AF_INET, &addr);
}
