#ifndef _pr_gc_hpp_
#define _pr_gc_hpp_

#include "prdefs.hpp"
#include "primitives.hpp"
#include <set>
#include <map>

namespace pr
{
    template<typename T> class RefCount;

    class GC
    {
    public:
        static void add_type(Type* type);

        static void register_instance(Type*, Object*);
        static void destroy_instance(Type*, Object*);

        static void add_root(Object* obj);
        static void del_root(Object* obj);

        // TODO: make temp roots O(1) (add and remove is structured)
        static void add_temp_root(Object* obj);
        static void del_temp_root(Object* obj);

        static void gc();
        static void force_gc();

        static int get_object_count();

        static inline void mark_alive(Object* obj)
        {
            obj->ref_count = (obj->ref_count & ~1) | alive_bit;
        }

        static inline bool is_marked(Object* obj)
        {
            return !!((obj->ref_count & 1) == alive_bit);
        }

        static inline bool is_in_progress()
        {
            return in_progress;
        }

        static inline void block_gc()
        {
            assert(blocked == false);
            blocked = true;
        }

        static inline void unblock_gc()
        {
            assert(blocked == true);
            blocked = false;
        }

        static inline void dec_ref(Object* p)
        {
            if (!in_progress)
                p->decrement_reference();
        }

        static inline void mark(Object* p)
        {
            assert(p);
            if (!GC::is_marked(p))
            {
                GC::mark_alive(p);
                p->gc_mark();
            }
        }

        template<typename T>
        static inline void mark(RefCount<T>& p)
        {
            if (p.get())
                mark(p.get());
        }

        static inline void mark(ObjP p)
        {
            if (is_object(p))
                mark(to_object(p));
        }

        static const std::set<Type*>& get_types() { return types; }

    private:
        static std::set<Type*> types;
        static std::map<Object*, int> roots;
        static int alive_bit;
        static bool in_progress;
        static bool blocked;
        static bool intensive_gc;
    };

    inline void inc_ref(ObjP p)
    {
        if (p && (p & 3) == 0)
            to_object(p)->increment_reference();
    }

    inline void inc_ref(Object* p)
    {
        if (p)
            p->increment_reference();
    }

    inline void dec_ref(ObjP p)
    {
        if (p && (p & 3) == 0)
            GC::dec_ref(to_object(p));
    }

    inline void dec_ref(Object* p)
    {
        if (p)
            GC::dec_ref(p);
    }

    template<typename T>
    class RefCount
    {
    public:
        RefCount() : obj(0) {}
        RefCount(T* t) : obj(0) { set(t); }
        ~RefCount() { set(0); }

        void set(T* t)
        {
            if (t)
                pr::inc_ref(t);

            if (obj)
                dec_ref(obj);

            obj = t;
        }

        T* get() { return obj; }

        T* operator->()
        {
            assert(obj);
            return obj;
        }

        void operator=(T* t)
        {
            set(t);
        }

        operator bool() const { return !!obj; }

    private:
        T* obj;
    };

    class LocalRef
    {
    public:
        LocalRef(Object* p) : obj(p)
        {
            if (obj)
                GC::add_temp_root(obj);
        }

        LocalRef(ObjP p) : obj(is_object(p) ? to_object(p) : 0)
        {
            if (obj)
                GC::add_temp_root(obj);
        }

        ~LocalRef()
        {
            if (obj)
                GC::del_temp_root(obj);
        }

    private:
        Object* obj;
    };

#define PUURO_CONCAT(a, b) a##b
#define PUURO_MAKE_TEMP(l) PUURO_CONCAT(_local_ref_, l)
#define PR_LOCAL_REF(var) LocalRef PUURO_MAKE_TEMP(__LINE__)(var);
}

#endif
