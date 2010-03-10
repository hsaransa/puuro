#include "resolver.hpp"
#include "type.hpp"
#include "string.hpp"
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
