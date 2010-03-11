#include "gc.hpp"
#include "type.hpp"
#include "selector.hpp"
#include <stdio.h>

using namespace pr;

std::set<Type*> GC::types;
std::map<Object*, int> GC::roots;
int GC::alive_bits;
int GC::alive_mask;
bool GC::in_progress;
bool GC::blocked;
bool GC::intensive_gc = true;

static std::map<Type*, std::set<Object*> > objects;

static int magic_counter;

void GC::add_type(Type* type)
{
    inc_ref(type);
    types.insert(type);
}

void GC::register_instance(Type* t, Object* o)
{
    assert(t && o);

    magic_counter++;

    assert(objects[t].count(o) == 0);
    objects[t].insert(o);

#if 0
    if (magic_counter >= 1024 || GC::intensive_gc)
    {
        PR_LOCAL_REF(o);
        GC::gc();
    }
#endif
}

void GC::destroy_instance(Type* t, Object* o)
{
    assert(t && o);

    if (!in_progress)
    {
        assert(objects[t].count(o) == 1);
        objects[t].erase(o);
        delete o;
    }
}

void GC::add_root(Object* obj)
{
    inc_ref(obj);
    roots[obj]++;
}

void GC::del_root(Object* obj)
{
    roots[obj]--;
    assert(roots[obj] >= 0);
    if (roots[obj] == 0)
        roots.erase(roots.find(obj));
    dec_ref(obj);
}

int GC::get_object_count()
{
    int n = 0;
    std::map<Type*, std::set<Object*> >::iterator iter;
    for (iter = objects.begin(); iter != objects.end(); iter++)
        n += iter->second.size();
    return n;
}

void GC::gc()
{
    if (magic_counter > 1024 || intensive_gc)
        GC::force_gc();
}

void GC::force_gc()
{
#ifdef NO_GC
    return;
#else
    if (in_progress || blocked)
        return;

    in_progress = true;

    magic_counter = 0;

    //printf("--- DOING GC %d\n", get_object_count());

    // Mark.

    alive_bits ^= 1;
    alive_mask = (1 << GC_BITS) - 1;

    get_selector()->gc_mark();

    {
        std::set<Type*>::iterator iter;
        for (iter = types.begin(); iter != types.end(); iter++)
            mark(*iter);
    }

    {
        std::map<Object*, int>::iterator iter;
        for (iter = GC::roots.begin(); iter != GC::roots.end(); iter++)
            mark(iter->first);
    }

    // Sweep.

    std::map<Type*, std::set<Object*> >::iterator iter;

    for (iter = objects.begin(); iter != objects.end(); iter++)
    {
        std::set<Object*>::iterator iter2;

        bool done;
        do
        {
            done = true;
            for (iter2 = iter->second.begin(); iter2 != iter->second.end(); iter2++)
            {
                if (!is_marked(*iter2))
                {
                    //Type* t = (*iter2)->get_type();
                    //printf("Collected %p with %s\n", *iter2, t->get_name().s());
                    delete *iter2;
                    iter->second.erase(iter2);
                    done = false;
                    break;
                }
            }
        } while (!done);
    }

    //printf("--- DONE HERE %d (%d types)\n", get_object_count(), types.size());

    in_progress = false;
#endif
}
