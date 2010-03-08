#include "std2.hpp"
#include "list.hpp"
#include "type.hpp"
#include "string.hpp"
#include "integer.hpp"
#include <std2.h>

using namespace pr;

/*
 * Std2
 */

Std2::Std2()
:   Object(get_type())
{
}

Std2::~Std2()
{
}

Type* Std2::get_type()
{
    static Type* type;
    if (!type)
    {
        type = new Type("std2");
        type->add_method("list_modules", (Callable::mptr0)&Std2::list_modules_);
        type->add_method("get_module", (Callable::mptr1)&Std2::get_module_);
    }

    return type;
}

ObjP Std2::list_modules_()
{
    int count = 0;
    std2_list_modules(0, &count);
    const char** names = new const char* [count];
    if (!names)
        throw new Exception("out_of_memory", 0);

    std2_list_modules(names, &count);

    List* l = new List();
    for (int i = 0; i < count; i++)
        l->append(name_to_symbol(Name(names[i])));

    delete [] names;

    return *l;
}

ObjP Std2::get_module_(ObjP s)
{
    if (!is_symbol(s))
        throw new Exception("bad_type", s);

    int ret = std2_find_module(symbol_to_name(s).s());
    if (ret < 0)
        throw new Exception("bad_module", s);

    return *new Std2Module(ret);
}

/*
 * Std2Module
 */

Std2Module::Std2Module(int m)
:   Object(get_type()), module(m)
{
}

Std2Module::~Std2Module()
{
}

Type* Std2Module::get_type()
{
    static Type* type;
    if (!type)
    {
        type = new Type("std2module");
        type->add_method("get_function", (Callable::mptr1)&Std2Module::get_function_);
    }
    return type;
}

ObjP Std2Module::get_function_(ObjP s)
{
    if (!is_symbol(s))
        throw new Exception("bad_type", s);

    int ret = std2_find_function(module, symbol_to_name(s).s());
    if (ret < 0)
        throw new Exception("bad_function", s);

    return *new Std2Function(module, ret);
}

/*
 * Std2Function
 */

Std2Function::Std2Function(int module, int function)
:   Object(get_type()), module(module), function(function)
{
    assert(module >= 0 && function >= 0);

    ret_type = std2_get_return_type(module, function);

    int n = std2_get_param_count(module, function);
    for (int i = 0; i < n; i++)
        params.push_back(std2_get_param_type(module, function, i));
}

Std2Function::~Std2Function()
{
}

Type* Std2Function::get_type()
{
    static Type* type;
    if (!type)
    {
        type = new Type("std2function");
        type->add_method("call", (Callable::mptrx)&Std2Function::call_);
    }
    return type;
}

ObjP Std2Function::call_(List* l)
{
#if 0
    if (l->get_size() != (int)params.size())
        throw new Exception("bad_param_count", *l);
#endif

    void*      args[16];
    std2_int32 int32s[16];
    std2_int64 int64s[16];

    assert(params.size() <= sizeof(args)/sizeof(args[0]));

    int j = 0;

    for (int i = 0; i < (int)params.size(); i++)
    {
        ObjP arg = l->get(j);

        switch (params[i].type)
        {
        case STD2_C_STRING:
            {
                String* s = to_string(arg);
                args[i] = (void*)s->get_data();
            }
            break;

        case STD2_INT32:
            {
                int32s[i] = int_value(arg);
                args[i] = &int32s[i];
            }
            break;

        case STD2_INT64:
            {
                int64s[i] = int_value(arg);
                args[i] = &int64s[i];
            }
            break;

        case STD2_BUF_PTR:
            {
                String* s = to_string(arg);
                args[i] = (void*)s->get_data();
                i++;
                int32s[i] = s->get_size();
                args[i] = &int32s[i];
            }
            break;

        case STD2_INSTANCE:
            {
                Object* obj = to_object(arg);
                Std2Instance* inst = dynamic_cast<Std2Instance*>(obj);
                if (!inst ||
                    inst->get_module() != params[i].module_id ||
                    inst->get_class() != params[i].class_id)
                {
                    throw new Exception("bad_type", arg);
                }

                args[i] = inst->get_ptr();
            }
            break;

        default:
            throw new Exception("unsupported_std2_type", int_to_fixnum(i));
        }

        j++;
    }

    switch (ret_type.type)
    {
    case STD2_INT32:
        {
            int ret;
            std2_call(module, function, (void*)&ret, args, params.size());
            return int_to_object(ret);
        }

    case STD2_INT64:
        {
            std2_int64 ret;
            std2_call(module, function, (void*)&ret, args, params.size());
            return *new Integer(ret);
        }

    case STD2_C_STRING:
        {
            const char* ret;
            std2_call(module, function, (void*)&ret, args, params.size());
            if (ret)
                return *new String(ret);
            else
                return 0;
        }

    case STD2_INSTANCE:
        {
            void* ret;
            std2_call(module, function, (void*)&ret, args, params.size());
            if (ret)
                return *new Std2Instance(ret_type.module_id, ret_type.class_id, ret);
            else
                return 0;
        }
        break;

    default:
        throw new Exception("unsupported_std2_type", 0);
    }

    return 0;
}

/*
 * Std2Instance
 */

Std2Instance::Std2Instance(int mod, int clas, void* ptr)
:   Object(get_type()), module(mod), clas(clas), ptr(ptr)
{
}

Std2Instance::~Std2Instance()
{
    std2_unrefer(module, clas, ptr);
}

Type* Std2Instance::get_type()
{
    static Type* type;
    if (!type)
    {
        type = new Type("std2instance");
    }
    return type;
}
