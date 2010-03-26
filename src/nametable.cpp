#include "nametable.hpp"

using namespace pr;

NameTable pr::g_nameTable;

namespace
{
    const char* standardIdToName(int id)
    {
        switch (id)
        {
#define MK_NAME(n) case N_##n: return #n;
            MK_NAME(Add)
            MK_NAME(Assign)
            MK_NAME(AssignMember)
            MK_NAME(AssignVariable)
            MK_NAME(AssignSinkMember)
            MK_NAME(AssignSinkVariable)
            MK_NAME(call)
            MK_NAME(Call)
            MK_NAME(CallMethod)
            MK_NAME(Code)
            MK_NAME(Div)
            MK_NAME(Eq)
            MK_NAME(ExprList)
            MK_NAME(GE)
            MK_NAME(GT)
            MK_NAME(Integer)
            MK_NAME(LE)
            MK_NAME(LT)
            MK_NAME(List)
            MK_NAME(Member)
            MK_NAME(Method)
            MK_NAME(Mod)
            MK_NAME(Mul)
            MK_NAME(NE)
            MK_NAME(Not)
            MK_NAME(Param)
            MK_NAME(ParamList)
            MK_NAME(ParamSink)
            MK_NAME(String)
            MK_NAME(Sub)
            MK_NAME(Symbol)
            MK_NAME(type)
            MK_NAME(Variable)
            MK_NAME(True)
            MK_NAME(False)
            MK_NAME(Null)
            MK_NAME(r)
            MK_NAME(w)
            MK_NAME(rw)
            MK_NAME(inet)
            MK_NAME(inet6)
            MK_NAME(unix)
            MK_NAME(stream)
            MK_NAME(dgram)
            MK_NAME(GetItem)
            MK_NAME(Neg)
            MK_NAME(And)
            MK_NAME(Or)
            MK_NAME(Float)
#undef MK_NAME
        }

        assert(!"if you get this assert you forgot to add standard name the list above");
        return 0;
    }
}

NameTable::NameTable()
{
    for (int i = 1; i < N__MAX; i++)
    {
        int id = getId(standardIdToName(i));
        (void)id;
        assert(id == i);
    }
}

NameTable::~NameTable()
{
}

int NameTable::getId(const char* name)
{
    std::map<std::string, int>::iterator iter;

    iter = nameToId.find(name);
    if (iter != nameToId.end())
        return iter->second;

    int id = (int)idToName.size() + 1;
    idToName.push_back(name);
    nameToId[name] = id;
    return id;
}

std::string NameTable::getName(int id) const
{
    assert(id > 0 && id <= (int)idToName.size());
    return idToName[id-1];
}
