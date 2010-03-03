#ifndef _pr_selector_hpp_
#define _pr_selector_hpp_

#include "prdefs.hpp"
#include "gc.hpp"
#include <map>
#include <queue>
#include <string.h>

struct pollfd; // POSIX stuff

namespace pr
{
    class Selector
    {
    public:
        static const int READ = 1;
        static const int WRITE = 1;
        static const int EXCEPT = 1;

        typedef void (*callback_func)(void*, ObjP);

        Selector();
        ~Selector();

        void gc_mark();

        bool empty();

        int64 get_current_time();

        void add_sleeper(int64 us, callback_func cb, void* user, ObjP obj);

        void select();

    private:
        struct Sleeper
        {
            int64 wake_time;
            callback_func callback;
            void* user;
            Ref<ObjP> obj;

            bool operator<(const Sleeper& b) const
            {
                if (wake_time == b.wake_time)
                    return memcmp(this, &b, sizeof(Sleeper));
                else
                    return wake_time < b.wake_time;
            }
        };

        std::vector<Sleeper> sleepers;

        struct pollfd* pollfds;
    };

    extern Selector g_selector;

    inline Selector* get_selector()
    {
        return &g_selector;
    }
}

#endif
