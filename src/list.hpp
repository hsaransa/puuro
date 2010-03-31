#ifndef _pr_list_hpp_
#define _pr_list_hpp_

#include "prdefs.hpp"
#include "gc.hpp"
#include "exception.hpp"

namespace pr
{
    class List : public Object
    {
    public:
        List();
        List(int n, ...);
        virtual ~List();

        virtual Type* get_type();
        virtual void gc_mark();
        virtual List* cast_list();

        List* copy() const;

        void append(ObjP p);
        void prepend(ObjP p);
        int get_size() { return (int)items.size(); }
        ObjP get(int i) { assert(i >= 0 && i < get_size()); return items[i]; }

        ObjP to_string_();
        ObjP map_(ObjP p);
        ObjP filter_(ObjP p);
        ObjP each_(ObjP p);
        ObjP append_(ObjP p);
        ObjP prepend_(ObjP p);
        ObjP extend_(ObjP p);
        ObjP pop_();
        ObjP pop_first_();
        ObjP empty_();
        ObjP size_();
        ObjP at_(ObjP);
        ObjP set_(ObjP, ObjP);
        ObjP add_(ObjP);
        ObjP first_();
        ObjP second_();
        ObjP third_();
        ObjP all_before_(ObjP);
        ObjP all_after_(ObjP);
        ObjP erase_at_(ObjP);

    private:
        std::vector<Ref<ObjP> > items;

        static Type* type;
    };

    inline List* to_list(ObjP p)
    {
        if (is_object(p))
            return to_object(p)->cast_list();
        throw new Exception("bad_type", p);
    }
}

#endif
