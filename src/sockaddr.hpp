#ifndef _pr_sockaddr_hpp_
#define _pr_sockaddr_hpp_

#include "prdefs.hpp"
#include "object.hpp"
#include <netinet/in.h>
#include <sys/un.h>

namespace pr
{
    class SockAddr : public Object
    {
    public:
        SockAddr(int family, void*);
        SockAddr(void*, int size);
        virtual ~SockAddr();

        virtual Type* get_type();

        struct sockaddr* get_sockaddr();
        int get_sockaddr_size();

    private:
        ObjP family_();
        ObjP to_string_();
        ObjP set_addr_(ObjP);
        ObjP set_port_(ObjP);
        ObjP a_();
        ObjP b_();
        ObjP c_();
        ObjP d_();

        union
        {
            struct sockaddr     addr;
            struct sockaddr_in  addr_in;
            struct sockaddr_in6 addr_in6;
            struct sockaddr_un  addr_un;
        };
    };
}

#endif
