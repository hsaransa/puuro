#ifndef _pr_std2_hpp_
#define _pr_std2_hpp_

#include "prdefs.hpp"
#include "object.hpp"
#include "gc.hpp"
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
        ObjP get_module_(ObjP s, ObjP f);
        ObjP get_main_fork_();
        ObjP fork_();
    };

    class Std2Fork : public Object
    {
    public:
        Std2Fork(int f);
        virtual ~Std2Fork();

        virtual Type* get_type();

        void invalidate() { valid = false; }

        bool is_valid() const { return valid; }
        int get_fork_id() const { return fork_id; }

    private:
        ObjP to_string_();
        ObjP valid_();

        bool valid;
        int fork_id;
    };

    class Std2Module : public Object
    {
    public:
        Std2Module(int m, Std2Fork*);
        virtual ~Std2Module();

        virtual Type* get_type();
        virtual void gc_mark();

    private:
        ObjP get_class_(ObjP);
        ObjP get_const_(ObjP);
        ObjP get_function_(ObjP);
        ObjP list_classes_();
        ObjP list_consts_();
        ObjP list_functions_();

        int module;
        Ref<Std2Fork*> fork;
    };

    class Std2Function : public Object
    {
    public:
        Std2Function(int mod, int func, Std2Fork*);
        virtual ~Std2Function();

        virtual Type* get_type();
        virtual void gc_mark();

    private:
        ObjP call_(List*);

        int module;
        int function;
        Ref<Std2Fork*> fork;

        std2_param ret_type;
        std::vector<std2_param> params;
    };

    class Std2Instance : public Object
    {
    public:
        Std2Instance(int mod, int clas, void* ptr, Std2Fork*);
        virtual ~Std2Instance();

        virtual Type* get_type();
        virtual void gc_mark();

        bool is_freed() { return freed; };
        int get_module() { return module; }
        int get_class() { return clas; }
        void* get_ptr() { return ptr; }
        Std2Fork* get_fork() { return fork.get(); }

    private:
        ObjP to_string_();
        ObjP get_fork_();
        ObjP free_();

        bool freed;
        int module;
        int clas;
        void* ptr;
        Ref<Std2Fork*> fork;
    };
}

#endif
