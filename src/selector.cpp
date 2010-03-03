#include "selector.hpp"
#include "exception.hpp"
#include "gc.hpp"
#include <sys/select.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>

using namespace pr;

Selector pr::g_selector;

Selector::Selector()
:   pollfds(0)
{
}

Selector::~Selector()
{
    free(pollfds);
}

void Selector::gc_mark()
{
    std::vector<Sleeper>::iterator iter;
    for (iter = sleepers.begin(); iter != sleepers.end(); iter++)
        GC::mark(iter->obj);
}

bool Selector::empty()
{
    return sleepers.empty();
}

int64 Selector::get_current_time()
{
    struct timeval tv;
    gettimeofday(&tv, 0);
    return (int64)tv.tv_sec * 1000000 + (int64)tv.tv_usec;
}

void Selector::add_sleeper(int64 us, callback_func cb, void* user, ObjP p)
{
    Sleeper s;
    s.wake_time = get_current_time() + us;
    s.callback = cb;
    s.user = user;
    s.obj = p;

    sleepers.push_back(s);

    pollfds = (struct pollfd*)realloc(pollfds, sleepers.size());
    if (!pollfds)
        throw new Exception("out_of_memory", 0);
}

void Selector::select()
{
    if (sleepers.empty())
        return;

    int best_i = 0;
    for (int i = 1; i < (int)sleepers.size(); i++)
        if (sleepers[i].wake_time < sleepers[best_i].wake_time)
            best_i = i;

    Sleeper s = sleepers[best_i];

    usleep(std::max((int64)0, s.wake_time - get_current_time()));

    sleepers.erase(sleepers.begin() + best_i);
    s.callback(s.user, s.obj);
}
