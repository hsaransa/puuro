#ifndef _pr_object_hpp_
#define _pr_object_hpp_

#include "prdefs.hpp"

namespace pr
{
    class Object
    {
        friend class GC;

    public:
        virtual Type* get_type()=0;
        virtual void  gc_mark();
        virtual int   mem_usage();

        virtual Integer*   cast_integer();
        virtual Code*      cast_code();
        virtual List*      cast_list();
        virtual String*    cast_string();
        virtual Exception* cast_exception();
        virtual Frame*     cast_frame();

        // TODO: don't increment if it would overflow
        void increment_reference() { ref_count += 1 << GC_BITS; }

        inline int get_ref_count() const { return ref_count >> GC_BITS; }

        inline operator ObjP()
        {
            return (ObjP)this;
        }

    protected:
        Object(Type*);
        virtual ~Object();

    private:
        // Deny copy and assignment.
        Object(const Object&) {}
        Object& operator=(const Object&) { return *this; }

        void destroy();

        // decrement_reference() is private for a reason, use dec_ref(obj) instead.
        // TODO: don't decrement if it has reached maximum value
        void decrement_reference() { ref_count -= 1 << GC_BITS; if (ref_count <= (1 << GC_BITS)-1) destroy(); }

    private:
        int ref_count; // NOTE: GC_BITS number of bits are reserved for GC
    };
}

#endif
