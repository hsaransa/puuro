#include "lexer.hpp"
#include "string.hpp"
#include "ast.hpp"
#include "integer.hpp"
#include "parser.hpp"
#include "exception.hpp"
#include "primitives.hpp"
#include "grammar.hh"
#include "gc.hpp"
#include "type.hpp"
#include <stdio.h>

using namespace pr;

Lexer::Lexer(Name f, String* s)
:   Object(get_type()),
    src(s),
    pos(0),
    object(0),
    file(f)
{
    assert(s);
    line = 1;
}

Lexer::~Lexer()
{
}

Type* Lexer::get_type()
{
    static Type* type;
    if (!type)
        type = new Type(Name("lexer"));
    return type;
}

void Lexer::gc_mark()
{
    GC::mark(src);
}

int Lexer::next()
{
    const char* data = src->get_data();
    int size = src->get_size();

    const char* p = data + pos;
    const char* end = data + size;

    current_token = 0;
    object = 0;

    while (p < end)
    {
        if (isspace(*p))
        {
            if (*p == '\n')
                line++;
            p++;
            continue;
        }

        if (*p == '#')
        {
            while (*p != '\n' && p < end)
                p++;
            if (*p == '\n')
            {
                line++;
                p++;
            }
            continue;
        }

        if (*p == '=' && *(p+1) == '=')
        {
            current_token = T_EQ;
            p += 2;
            break;
        }
        else if (*p == '!' && *(p+1) == '=')
        {
            current_token = T_NE;
            p += 2;
            break;
        }
        else if (*p == '<' && *(p+1) == '=')
        {
            current_token = T_LE;
            p += 2;
            break;
        }
        else if (*p == '>' && *(p+1) == '=')
        {
            current_token = T_GE;
            p += 2;
            break;
        }
        else if (*p == '\\' && *(p+1) == 't')
        {
            current_token = T_TRUE;
            p += 2;
            if (isalpha(*p) || *p == '_')
                throw new Exception(Name("lexer_error"), int_to_fixnum(get_line()));
            break;
        }
        else if (*p == '\\' && *(p+1) == 'f')
        {
            current_token = T_FALSE;
            p += 2;
            if (isalpha(*p) || *p == '_')
                throw new Exception(Name("lexer_error"), int_to_fixnum(get_line()));
            break;
        }
        else if (strchr("(){}[]<>:;,=+-*/.'!%|&", *p))
        {
            current_token = *p;
            p++;
            break;
        }
        else if (isalpha(*p) || *p == '_')
        {
            const char* s = p;
            while ((isalnum(*p) || *p == '_') && p < end)
                p++;

            std::string ss = std::string(s, p);

            current_token = T_IDENTIFIER;
            object = name_to_symbol(ss);
            break;

        }
        else if (isdigit(*p))
        {
            const char* s = p;
            while (isdigit(*p) && p < end)
                p++;

            current_token = T_INTEGER;
            object = (ObjP)*new Integer(s);
            dec_ref(object);
            break;
        }
        else if (*p == '"')
        {
            p++;
            const char* s = p;
            while (*p != '"' && p < end)
                p++;

            current_token = T_STRING;
            object = (ObjP)*new String(s, p - s);
            dec_ref(object);

            if (p < end)
                p++;
            else
                throw new Exception(Name("lexer_error"), int_to_fixnum(get_line()));

            break;
        }
        else
        {
            throw new Exception(Name("lexer_error"), int_to_fixnum(get_line()));
        }
    }

    pos = p - data;

    yylval.obj = object;
    return current_token;
}
