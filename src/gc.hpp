#ifndef _pr_gc_hpp_
#define _pr_gc_hpp_

#include "prdefs.hpp"
#include "primitives.hpp"
#include <set>
#include <map>

namespace pr
{
    class GC
    {
    public:
        static void add_type(Type* type);

        static void register_instance(Type*, Object*);
        static void destroy_instance(Type*, Object*);

        static void add_root(Object* obj);
        static void del_root(Object* obj);

        static void gc();
        static void force_gc();

        static int get_object_count();

        static inline void mark_alive(Object* obj)
        {
            obj->ref_count = (obj->ref_count & ~alive_mask) | alive_bits;
        }

        static inline bool is_marked(Object* obj)
        {
            return !!((obj->ref_count & alive_mask) == alive_bits);
        }

        static inline bool is_in_progress()
        {
            return in_progress;
        }

        static inline void dec_ref(Object* p)
        {
            if (!in_progress)
            {
                assert(p->get_ref_count() > 0);
                p->decrement_reference();
            }
        }

        static inline void mark(Object* p)
        {
            assert(p); // NULL is asserted instead of checked because it's
                       // it's very rare that p is NULL
#ifdef OUTPUT_GC_GRAPH
            gc_graph_out << "n" << gc_parent << " -> n" << p << '\n';
#endif
            if (!GC::is_marked(p))
            {
                GC::mark_alive(p);
                p->gc_mark();
            }
        }

        template<typename T>
        static inline void mark(Ref<T>& p)
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
        static int alive_bits;
        static int alive_mask;
        static bool in_progress;
        static bool intensive_gc;
    };

    inline ObjP inc_ref(ObjP p)
    {
#ifndef NO_REFCOUNT
        if (p && (p & 3) == 0)
            to_object(p)->increment_reference();
#endif
        return p;
    }

    inline Object* inc_ref(Object* p)
    {
#ifndef NO_REFCOUNT
        if (p)
            p->increment_reference();
#endif
        return p;
    }

    inline void dec_ref(ObjP p)
    {
        (void)p;
#ifndef NO_REFCOUNT
        if (p && (p & 3) == 0)
            GC::dec_ref(to_object(p));
#endif
    }

    inline void dec_ref(Object* p)
    {
        (void)p;
#ifndef NO_REFCOUNT
        if (p)
            GC::dec_ref(p);
#endif
    }

#define return_and_inc_ref(p) return inc_ref(p), p

    template<typename T>
    class Ref
    {
    public:
        Ref() : obj(0) {}
        Ref(const Ref<T>& t) : obj(0) { set(t.get()); }
        Ref(const T& t) : obj(0) { set(t); }
        ~Ref() { set(0); }

        void set(T t)
        {
            inc_ref(t);
            T old = obj;
            obj = t;
            dec_ref(old);
        }

        T get() { return obj; }
        const T get() const { return obj; }

        T operator->()
        {
            assert(obj);
            return obj;
        }

        const T operator->() const
        {
            assert(obj);
            return obj;
        }

        operator T() const { return obj; }

        void operator=(const T& t)
        {
            set(t);
        }

        void operator=(const Ref<T>& t)
        {
            set(t.get());
        }

    private:
        T obj;
    };

#if 0
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

        LocalRef(const LocalRef&) {}
        void operator=(const LocalRef&) {}
    };

#define PUURO_CONCAT(a, b) a##b
#define PUURO_MAKE_TEMP(l) PUURO_CONCAT(_local_ref_, l)
#define PR_LOCAL_REF(var) LocalRef PUURO_MAKE_TEMP(__LINE__)(var);
#endif
}

#endif
