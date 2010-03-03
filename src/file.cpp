#include "file.hpp"
#include "exception.hpp"
#include "string.hpp"
#include "type.hpp"
#include <string.h>
#include <stdio.h>

using namespace pr;

File::File(const char* fn)
:   Object(get_type()), filename(fn)
{
}

File::~File()
{
}

Type* File::get_type()
{
    static Type* type;
    if (!type)
        type = new Type(Name("file"));
    return type;
}

String* File::read_file()
{
    FILE* fp = fopen(filename.c_str(), "rb");
    if (!fp)
        throw new Exception(Name("file_lolled"), *new String(filename.c_str()));

    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char* tmp = new char [size];

    int n = fread(tmp, size, 1, fp);
    if (n <= 0)
    {
        delete [] tmp;
        throw new Exception(Name("file_lolled_2"), *new String(filename.c_str()));
    }
    fclose(fp);

    String* str = new String(tmp, size);
    delete [] tmp;

    return str;
}
