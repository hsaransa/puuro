#ifndef _pr_string_hpp_
#define _pr_string_hpp_

#include "prdefs.hpp"
#include "exception.hpp"

namespace pr
{
    class String : public Object
    {
    public:
        String();
        String(const char* p);
        String(const char* p, int size);
        virtual ~String();

        virtual Type* get_type();
        virtual String* cast_string();

        String* copy() const;
        const char* get_data() const;
        int get_size() const;

        void append(String* s);
        void append(const char* s);

    private:
        ObjP add(ObjP);
        ObjP at(ObjP);
        ObjP len_();
        ObjP eq_(ObjP p);
        ObjP ne_(ObjP p);
        ObjP lt_(ObjP p);
        ObjP find_(ObjP p);
        ObjP all_before_(ObjP);
        ObjP all_after_(ObjP);
        ObjP last_();
        ObjP to_symbol_();
        ObjP to_integer_();
        ObjP ord_(ObjP);

    private:
        std::string data;
    };

    inline String* to_string(ObjP p)
    {
        if (is_object(p))
            return to_object(p)->cast_string();
        throw new Exception("bad_type", p);
    }
}

#endif
