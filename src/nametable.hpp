#ifndef _pr_nametable_hpp_
#define _pr_nametable_hpp_

#include "prdefs.hpp"
#include <map>
#include <vector>
#include <string>
#include <cassert>
#include <string.h>

namespace pr
{
    // Constant name ids to allow switching.

    enum
    {
        N_Add = 1,
        N_Assign,
        N_AssignMember,
        N_AssignVariable,
        N_call,
        N_Call,
        N_CallMethod,
        N_Code,
        N_Div,
        N_Eq,
        N_ExprList,
        N_GE,
        N_GT,
        N_Integer,
        N_LE,
        N_LT,
        N_List,
        N_Member,
        N_Method,
        N_Mod,
        N_Mul,
        N_NE,
        N_Not,
        N_Param,
        N_ParamList,
        N_ParamSink,
        N_String,
        N_Sub,
        N_Symbol,
        N_type,
        N_Variable,
        N_True,
        N_False,
        N_Null,
        N_r,
        N_w,
        N_rw,
        N_unix,
        N_inet,
        N_inet6,
        N_stream,
        N_dgram,
        N_GetItem,
        N__MAX
    };

    // NameTable.

    class NameTable
    {
    public:
        NameTable();
        ~NameTable();

        int getId(const char*);
        std::string getName(int) const;

    private:
        std::vector<std::string> idToName;
        std::map<std::string, int> nameToId;
    };

    extern NameTable g_nameTable;

    // Helper class to use g_nameTable.

    class Name
    {
    public:
        Name() : _id(0)
        {
        }

        Name(const char* s)
        {
            _id = g_nameTable.getId(s);
        }

        Name(const std::string& s)
        {
            _id = g_nameTable.getId(s.c_str());
        }

        explicit Name(int id) : _id(id)
        {
            assert(id > 0);
        }

        ~Name()
        {
        }

        int id() const
        {
            return _id;
        }

        bool valid() const
        {
            assert(_id >= 0);
            return _id != 0;
        }

        std::string name() const
        {
            return g_nameTable.getName(_id);
        }

        const char* s() const
        {
            static char buf[64];
            strncpy(buf, g_nameTable.getName(_id).c_str(), 64);
            return buf;
        }

        bool operator==(const Name& b) const
        {
            return _id == b._id;
        }

        bool operator<(const Name& b) const
        {
            return _id < b._id;
        }

    private:
        int _id;
    };

}

#endif
