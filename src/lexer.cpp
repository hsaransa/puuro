#include "lexer.hpp"
#include "integer.hpp"
#include "grammar.hh"
#include "type.hpp"
#include "float.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
            p++;
            if (*p == '-')
            {
                p++;
                while (p < end)
                {
                    if (*p == '\n')
                        line++;
                    if (*p == '-' && *(p+1) == '#')
                    {
                        p += 2;
                        break;
                    }
                    p++;
                }
            }
            else
            {
                while (*p != '\n' && p < end)
                    p++;
                if (*p == '\n')
                {
                    line++;
                    p++;
                }
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
        else if (*p == '.' && *(p+1) == '.')
        {
            current_token = T_TWO_DOTS;
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
        else if (*p == '\\' && *(p+1) == 'n')
        {
            current_token = T_NULL;
            p += 2;
            if (isalpha(*p) || *p == '_')
                throw new Exception(Name("lexer_error"), int_to_fixnum(get_line()));
            break;
        }
        else if (strchr("(){}[]<>:;,=+-*/'!%|&", *p))
        {
            if (*p == '(' && *(p+1) == ')')
            {
                p++;
                current_token = T_EMPTY_PARENS;
            }
            else
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
        else if (isdigit(*p) || *p == '.')
        {
            const char* s = p;
            while (isdigit(*p) && p < end)
                p++;

            if (*p == '.' && !(isalpha(*(p+1)) || *(p+1) == '_'))
            {
                p++;
                while (isdigit(*p) && p < end)
                    p++;

                if (p - s == 1)
                {
                    current_token = '.';
                    break;
                }

                double v = strtod(s, (char**)&p);
                if (s == p)
                    throw new Exception(Name("lexer_error"), int_to_fixnum(get_line()));

                object = (ObjP)*new Float(v);
                dec_ref(object);
                current_token = T_FLOAT;
            }
            else
            {
                if (*p == '.' && p - s == 0)
                {
                    p++;
                    current_token = '.';
                    break;
                }

                current_token = T_INTEGER;

                long int i = strtol(s, 0, 10);
                if (does_fit_fixnum(i))
                    object = int_to_fixnum(i);
                else
                    object = (ObjP)*new Integer(s);
                dec_ref(object);
            }
            break;
        }
        else if (*p == '"')
        {
            p++;

            std::string ss;

            while (*p != '"' && p < end)
            {
                if (*p == '\\')
                {
                    p++;
                    if (*p == 'n')
                        ss += '\n';
                    else if (*p == 'r')
                        ss += '\r';
                    else if (*p == 't')
                        ss += '\t';
                    else if (*p == '\\')
                        ss += '\\';
                    else if (*p == '"')
                        ss += '"';
                    else
                    {
                        ss += '\\';
                        ss += *p;
                    }
                }
                else
                    ss += *p;

                p++;
            }

            current_token = T_STRING;
            object = (ObjP)*new String(ss.c_str(), ss.length());
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

String* pr::read_file(const char* fn)
{
    FILE* fp = fopen(fn, "rb");
    if (!fp)
        throw new Exception(Name("file_lolled"), *new String(fn));

    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char* tmp = new char [size];

    int n = fread(tmp, size, 1, fp);
    fclose(fp);

    if (n <= 0)
    {
        delete [] tmp;
        throw new Exception(Name("file_lolled_2"), *new String(fn));
    }

    String* str = new String(tmp, size);
    delete [] tmp;

    return str;
}
