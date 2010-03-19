#ifndef _pr_selector_hpp_
#define _pr_selector_hpp_

#include "prdefs.hpp"
#include "gc.hpp"

struct pollfd; // POSIX stuff

namespace pr
{
    class Selector
    {
    public:
        static const int READ  = 1;
        static const int WRITE = 2;
        static const int ERROR = 4;

        typedef void (*sleeper_callback_func)(void*, ObjP);
        typedef void (*watcher_callback_func)(int fd, int mask, void*, ObjP);

        Selector();
        ~Selector();

        void gc_mark();

        bool empty();

        int64 get_current_time();

        void add_sleeper(int64 us, sleeper_callback_func cb, void* user, ObjP obj);
        void add_watcher(int fd, int mask, watcher_callback_func cb, void* user, ObjP obj);

        void clean_fd(int fd);

        void select();

    private:
        struct Sleeper
        {
            int64 wake_time;
            sleeper_callback_func callback;
            void* user;
            Ref<ObjP> obj;

#if 0
            bool operator<(const Sleeper& b) const
            {
                if (wake_time == b.wake_time)
                    return memcmp(this, &b, sizeof(Sleeper));
                else
                    return wake_time < b.wake_time;
            }
#endif
        };

        struct Watcher
        {
            int fd;
            int mask;
            watcher_callback_func callback;
            void* user;
            Ref<ObjP> obj;
        };

        std::vector<Sleeper> sleepers;
        std::vector<Watcher> watchers;

        struct pollfd* pollfds;
    };

    extern Selector g_selector;

    inline Selector* get_selector()
    {
        return &g_selector;
    }
}

#endif
