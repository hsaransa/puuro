#include "std2.hpp"
#include "list.hpp"
#include "type.hpp"
#include "integer.hpp"
#include "selector.hpp"
#include "frame.hpp"
#include "float.hpp"
#include <stdio.h>
#include <stdlib.h>

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
        type->add_method("get_const", (Callable::mptr1)&Std2Module::get_const_);
        type->add_method("list_classes", (Callable::mptr0)&Std2Module::list_classes_);
        type->add_method("list_consts", (Callable::mptr0)&Std2Module::list_consts_);
        type->add_method("list_functions", (Callable::mptr0)&Std2Module::list_functions_);
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

ObjP Std2Module::get_const_(ObjP s)
{
    if (!is_symbol(s))
        throw new Exception("bad_type", s);

    int ret = std2_find_const(module, symbol_to_name(s).s());
    if (ret < 0)
        throw new Exception("bad_function", s);

    enum std2_const_type t = std2_get_const_type(module, ret);
    const void* ptr = std2_get_const(module, ret);
    switch (t)
    {
    case STD2_CONST_INT: return int_to_object(*(const int*)ptr);
    default:
        return 0;
    }
}

#define LIST_ITEMS(f) \
    int count = 0; \
    f(module, 0, &count); \
    const char** names = new const char* [count]; \
    if (!names) \
        throw new Exception("out_of_memory", 0); \
\
    f(module, names, &count); \
\
    List* l = new List(); \
    for (int i = 0; i < count; i++) \
        l->append(name_to_symbol(Name(names[i]))); \
\
    delete [] names; \
\
    return *l;

ObjP Std2Module::list_classes_()
{
    LIST_ITEMS(std2_list_classes)
}

ObjP Std2Module::list_consts_()
{
    LIST_ITEMS(std2_list_consts)
}

ObjP Std2Module::list_functions_()
{
    LIST_ITEMS(std2_list_functions)
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

struct callback_data
{
    struct std2_callback cb;
    struct std2_param    ret_type;
};

static void callback(int fd, int mask, void* user, ObjP p)
{
    callback_data* data = (callback_data*)user;

    int m = 0;
    if (mask & Selector::READ)
        m |= STD2_CALLBACK_READ;
    if (mask & Selector::WRITE)
        m |= STD2_CALLBACK_WRITE;
    if (mask & Selector::ERROR)
        m |= STD2_CALLBACK_ERROR;
    if (mask & Selector::ABORT)
        m |= STD2_CALLBACK_ABORT;

    assert(data->cb.fd == fd);
    assert((data->cb.flags & m) != 0);

    // Call std2.

    int ret;
    ObjP ret_obj = 0;

    switch (data->ret_type.type)
    {
    case STD2_INT32:
        {
            int value = 0;
            ret = std2_call_callback(&data->cb, (void*)&value, m);
            if (!ret)
                ret_obj = int_to_object(value);
            break;
        }

    case STD2_INT64:
        {
            std2_int64 value = 0;
            ret = std2_call_callback(&data->cb, (void*)&value, m);
            if (!ret)
                ret_obj = *new Integer(value);
            break;
        }

    case STD2_C_STRING:
        {
            const char* value = 0;
            ret = std2_call_callback(&data->cb, (void*)&value, m);
            if (!ret && value)
                ret_obj = *new String(value);
            break;
        }

    case STD2_M_STRING:
        {
            char* value = 0;
            ret = std2_call_callback(&data->cb, (void*)&value, m);
            if (!ret && value)
                ret_obj = *new String(value);
            free(value);
            break;
        }

    case STD2_INSTANCE:
        {
            void* value = 0;
            ret = std2_call_callback(&data->cb, (void*)&value, m);
            if (!ret && value)
                ret_obj = *new Std2Instance(data->ret_type.module_id, data->ret_type.class_id, value);
            break;
        }

    default:
        throw new Exception("unsupported_std2_type", 0);
    }

    // Handle callback.

    if (ret)
    {
        struct std2_callback cb = std2_get_callback();

        int m = 0;
        if (cb.flags & STD2_CALLBACK_READ)
            m |= Selector::READ;
        if (cb.flags & STD2_CALLBACK_WRITE)
            m |= Selector::WRITE;
        if (cb.flags & STD2_CALLBACK_ERROR)
            m |= Selector::ERROR;
        if (cb.flags & STD2_CALLBACK_ABORT)
            m |= Selector::ABORT;

        data->cb = cb;

        get_selector()->add_watcher(cb.fd, m, callback, data, p);

        assert(ret_obj == 0);

        return;
    }

    delete data;

    // Return to caller frame.

    Frame* f = to_frame(p);
    f->push(ret_obj);
    get_executor()->set_frame(f);
}

ObjP Std2Function::call_(List* l)
{
    // Process parameters.
    // TODO: use alloca()? was there some problem with it?

    void*       args[16];
    std2_int32  int32s[16];
    std2_int64  int64s[16];
    float       floats[16];
    double      doubles[16];
    std2_buffer buffers[16];

    assert(params.size() <= sizeof(args)/sizeof(args[0]));

    int j = 0;

    for (int i = 0; i < (int)params.size(); i++)
    {
        if (j >= l->get_size())
            throw new Exception("bad_argument_count", *l);

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
            int32s[i] = int_value(arg);
            args[i] = &int32s[i];
            break;

        case STD2_INT64:
            int64s[i] = int_value(arg);
            args[i] = &int64s[i];
            break;

        case STD2_FLOAT:
            floats[i] = float_value(arg);
            args[i] = &floats[i];
            break;

        case STD2_DOUBLE:
            doubles[i] = float_value(arg);
            args[i] = &doubles[i];
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

        case STD2_C_BUFFER:
            {
                String* s = to_string(arg);
                buffers[i].data = (void*)s->get_data();
                buffers[i].size = s->get_size();
                args[i] = &buffers[i];
            }
            break;

        case STD2_INSTANCE:
            {
                Std2Instance* inst = cast_object<Std2Instance*>(arg);
                if (!inst ||
                    inst->get_module() != params[i].module_id ||
                    inst->get_class() != params[i].class_id)
                {
                    throw new Exception("bad_type", arg);
                }

                if (inst->is_freed())
                    throw new Exception("instance_freed", arg);

                args[i] = inst->get_ptr();
            }
            break;

        default:
            throw new Exception("unsupported_std2_type", int_to_fixnum(i));
        }

        j++;
    }

    if (j != l->get_size())
        throw new Exception("bad_argument_count", *l);

    // Call std2.

    int ret;
    ObjP ret_obj = 0;

    switch (ret_type.type)
    {
    case STD2_VOID:
        ret = std2_call(module, function, 0, args);
        break;

    case STD2_INT32:
        {
            int value = 0;
            ret = std2_call(module, function, (void*)&value, args);
            if (!ret)
                ret_obj = int_to_object(value);
            break;
        }

    case STD2_INT64:
        {
            std2_int64 value = 0;
            ret = std2_call(module, function, (void*)&value, args);
            if (!ret)
                ret_obj = *new Integer(value);
            break;
        }

    case STD2_C_STRING:
        {
            const char* value = 0;
            ret = std2_call(module, function, (void*)&value, args);
            if (!ret && value)
                ret_obj = *new String(value);
            break;
        }

    case STD2_M_STRING:
        {
            char* value = 0;
            ret = std2_call(module, function, (void*)&value, args);
            if (!ret && value)
                ret_obj = *new String(value);
            free(value);
            break;
        }

    case STD2_M_BUFFER:
    case STD2_C_BUFFER:
        {
            struct std2_buffer value;
            ret = std2_call(module, function, (void*)&value, args);
            if (!ret && value.data)
                ret_obj = *new String((const char*)value.data, value.size);
            if (ret_type.type == STD2_M_BUFFER)
                free(value.data);
            break;
        }

    case STD2_INSTANCE:
        {
            void* value = 0;
            ret = std2_call(module, function, (void*)&value, args);
            if (!ret && value)
                ret_obj = *new Std2Instance(ret_type.module_id, ret_type.class_id, value);
            break;
        }

    default:
        throw new Exception("unsupported_std2_type", 0);
    }

    // Handle callback.

    if (ret)
    {
        struct std2_callback cb = std2_get_callback();

        int m = 0;
        if (cb.flags & STD2_CALLBACK_READ)
            m |= Selector::READ;
        if (cb.flags & STD2_CALLBACK_WRITE)
            m |= Selector::WRITE;
        if (cb.flags & STD2_CALLBACK_ERROR)
            m |= Selector::ERROR;
        if (cb.flags & STD2_CALLBACK_ABORT)
            m |= Selector::ABORT;

        callback_data* data = new callback_data;
        data->ret_type = ret_type;
        data->cb = cb;

        Frame* f = get_executor()->get_frame();
        get_selector()->add_watcher(cb.fd, m, callback, data, *f);

        get_executor()->set_frame(0);

        assert(ret_obj == 0);
        ret_obj = error_object();
    }

    return ret_obj;
}

/*
 * Std2Instance
 */

Std2Instance::Std2Instance(int mod, int clas, void* ptr)
:   Object(get_type()), freed(false), module(mod), clas(clas), ptr(ptr)
{
}

Std2Instance::~Std2Instance()
{
    if (!freed)
        std2_unrefer(module, clas, ptr);
}

Type* Std2Instance::get_type()
{
    static Type* type;
    if (!type)
    {
        type = new Type("std2instance");
        type->add_method("to_string", (Callable::mptr0)&Std2Instance::to_string_);
        type->add_method("free", (Callable::mptr0)&Std2Instance::free_);
    }
    return type;
}

ObjP Std2Instance::to_string_()
{
    return *new String("<std2instance>");
}

ObjP Std2Instance::free_()
{
    if (!freed)
    {
        std2_unrefer(module, clas, ptr);
        freed = true;
    }
    return 0;
}
