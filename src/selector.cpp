#include "selector.hpp"
#include "exception.hpp"
#include "gc.hpp"
#include <sys/select.h>
#include <sys/time.h>
#include <poll.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

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
    {
        std::vector<Sleeper>::iterator iter;
        for (iter = sleepers.begin(); iter != sleepers.end(); iter++)
            GC::mark(iter->obj);
    }
    {
        std::vector<Watcher>::iterator iter;
        for (iter = watchers.begin(); iter != watchers.end(); iter++)
            GC::mark(iter->obj);
    }
}

bool Selector::empty()
{
    return sleepers.empty() && watchers.empty();
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
}

void Selector::add_watcher(int fd, int mask, callback_func cb, void* user, ObjP p)
{
    Watcher w;
    w.fd = fd;
    w.mask = mask;
    w.callback = cb;
    w.user = user;
    w.obj = p;

    watchers.push_back(w);

    pollfds = (struct pollfd*)realloc(pollfds, watchers.size() * sizeof(struct pollfd));
    if (!pollfds)
        throw new Exception("out_of_memory", 0);
}

void Selector::clean_fd(int fd)
{
    for (int i = 0; i < (int)watchers.size(); )
    {
        if (watchers[i].fd == fd)
            watchers.erase(watchers.begin() + i);
        else
            i++;
    }
}

void Selector::select()
{
    Sleeper sleeper;
    int best_i = -1;
    int timeout = -1;

    if (!sleepers.empty())
    {
        best_i = 0;
        for (int i = 1; i < (int)sleepers.size(); i++)
            if (sleepers[i].wake_time < sleepers[best_i].wake_time)
                best_i = i;

        sleeper = sleepers[best_i];

        timeout = (int)(std::max((int64)0, sleeper.wake_time - get_current_time()) / 1000);
    }

    for (int i = 0; i < (int)watchers.size(); i++)
    {
        pollfds[i].fd = watchers[i].fd;
        pollfds[i].events = 0;
        if (watchers[i].mask & READ)
            pollfds[i].events |= POLLIN;
        if (watchers[i].mask & WRITE)
            pollfds[i].events |= POLLOUT;
        if (watchers[i].mask & ERROR)
            pollfds[i].events |= POLLERR;
    }

    if (timeout < 0 && watchers.empty())
        return;

    int n = poll(pollfds, watchers.size(), timeout);

    if (n < 0)
        throw new Exception("system_error", int_to_fixnum(errno));

    if (n == 0)
    {
        if (best_i >= 0)
        {
            sleepers.erase(sleepers.begin() + best_i);
            sleeper.callback(sleeper.user, sleeper.obj);
        }

        return;
    }

    for (int i = 0; i < (int)watchers.size(); i++)
    {
        if (!pollfds[i].revents)
            continue;

        Watcher& w = watchers[i];

        w.callback(w.user, w.obj);

        watchers.erase(watchers.begin() + i);
        break;
    }
}
