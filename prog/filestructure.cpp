#include "filestructure.h"

fileStructure::fileStructure()
{

}

uint fileStructure::leb128u(int ptr)
{
    uint result = 0;
    int shift = 0;
    leb128Size = 0;
    while (true)
    {
        result |= (raw[ptr] & 0b01111111) << shift;
        leb128Size++;
        if (!(raw[ptr] & 0b10000000))
        {
            break;
        }
        shift += 7;
        ptr++;
    }
    return result;
}

sint fileStructure::leb128s(int ptr, int size)
{
    sint result = 0;
    int shift = 0;
    leb128Size = 0;
    do
    {
        result |= ((raw[ptr] & 0b01111111) << shift);
        shift += 7;
        ptr++;
        leb128Size++;
    }
    while ((raw[ptr - 1] & 0b10000000));

    if ((shift < size) && (raw[ptr - 1] & 0b01000000))
    {
        result |= (~0 << shift);
    }

    return result;
}

std::string fileStructure::f32tostr(int ptr)
{
    union
    {
        uchar union_raw[4];
        float union_val;
    };
    union_raw[0] = raw[ptr + 0];
    union_raw[1] = raw[ptr + 1];
    union_raw[2] = raw[ptr + 2];
    union_raw[3] = raw[ptr + 3];
    return hex::singleToStr(union_val);
}

std::string fileStructure::f64tostr(int ptr)
{
    union
    {
        uchar union_raw[8];
        double union_val;
    };
    union_raw[0] = raw[ptr + 0];
    union_raw[1] = raw[ptr + 1];
    union_raw[2] = raw[ptr + 2];
    union_raw[3] = raw[ptr + 3];
    union_raw[4] = raw[ptr + 4];
    union_raw[5] = raw[ptr + 5];
    union_raw[6] = raw[ptr + 6];
    union_raw[7] = raw[ptr + 7];
    return hex::doubleToStr(union_val);
}

uint fileStructure::intu(int ptr)
{
    union
    {
        uchar union_raw[4];
        uint union_val;
    };
    union_raw[0] = raw[ptr + 0];
    union_raw[1] = raw[ptr + 1];
    union_raw[2] = raw[ptr + 2];
    union_raw[3] = raw[ptr + 3];
    return union_val;
}

sint fileStructure::ints(int ptr)
{
    union
    {
        uchar union_raw[4];
        sint union_val;
    };
    union_raw[0] = raw[ptr + 0];
    union_raw[1] = raw[ptr + 1];
    union_raw[2] = raw[ptr + 2];
    union_raw[3] = raw[ptr + 3];
    return union_val;
}
