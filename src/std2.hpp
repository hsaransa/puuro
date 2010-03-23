#ifndef _pr_std2_hpp_
#define _pr_std2_hpp_

#include "prdefs.hpp"
#include "object.hpp"
#include <vector>
#include <std2.h>

namespace pr
{
    class Std2 : public Object
    {
    public:
        Std2();
        virtual ~Std2();

        virtual Type* get_type();

    private:
        ObjP list_modules_();
        ObjP get_module_(ObjP s);
    };

    class Std2Module : public Object
    {
    public:
        Std2Module(int m);
        virtual ~Std2Module();

        virtual Type* get_type();

    private:
        ObjP get_class_(ObjP);
        ObjP get_const_(ObjP);
        ObjP get_function_(ObjP);
        ObjP list_classes_();
        ObjP list_consts_();
        ObjP list_functions_();

        int module;
    };

    class Std2Function : public Object
    {
    public:
        Std2Function(int mod, int func);
        virtual ~Std2Function();

        virtual Type* get_type();

    private:
        ObjP call_(List*);

        int module;
        int function;

        std2_param ret_type;
        std::vector<std2_param> params;
    };

    class Std2Instance : public Object
    {
    public:
        Std2Instance(int mod, int clas, void* ptr);
        virtual ~Std2Instance();

        virtual Type* get_type();

        bool is_freed() { return freed; };
        int get_module() { return module; }
        int get_class() { return clas; }
        void* get_ptr() { return ptr; }

    private:
        ObjP to_string_();
        ObjP free_();

        bool freed;
        int module;
        int clas;
        void* ptr;
    };
}

#endif
