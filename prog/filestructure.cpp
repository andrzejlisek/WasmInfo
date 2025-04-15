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

sllong fileStructure::leb128s(int ptr, int size)
{
    // Measure number length
    leb128Size = 0;
    while (raw[ptr + leb128Size] & 0b10000000)
    {
        leb128Size++;
    }
    leb128Size++;


    // Longer number - uses two 64-bit numbers as teporary
    if (leb128Size >= 8)
    {
        sllong result_h = 0;
        sllong result_l = 0;
        int shift = 0;

        int bits_l = 56;
        int bits_h = 63 - bits_l;
        do
        {
            if (shift < bits_l)
            {
                result_l |= ((sllong)(raw[ptr] & 0b01111111) << shift);
            }
            else
            {
                result_h |= ((sllong)(raw[ptr] & 0b01111111) << (shift - bits_l));
            }
            shift += 7;
            ptr++;
        }
        while ((raw[ptr - 1] & 0b10000000));

        if ((shift < size) && (raw[ptr - 1] & 0b01000000))
        {
            if (shift < bits_l)
            {
                result_l |= (~0ull << shift);
                result_h |= (~0ull);
            }
            else
            {
                result_h |= (~0ull << (shift - bits_l));
            }

            if ((~result_h) >> bits_h)
            {
                return INT64_MIN;
            }

            result_h = (~result_h) & ((1ll << bits_h) - 1ll);
            result_l = (~result_l) & ((1ll << bits_l) - 1ll);
            return ~((result_h << bits_l) | result_l);
        }
        else
        {
            if (result_h >> bits_h)
            {
                return INT64_MAX;
            }

            result_h = result_h & ((1ll << bits_h) - 1ll);
            result_l = result_l & ((1ll << bits_l) - 1ll);
            return (result_h << bits_l) | result_l;
        }
    }
    else
    {
        sllong result = 0;
        int shift = 0;

        do
        {
            result |= ((sllong)(raw[ptr] & 0b01111111) << shift);
            shift += 7;
            ptr++;
        }
        while ((raw[ptr - 1] & 0b10000000));

        if ((shift < size) && (raw[ptr - 1] & 0b01000000))
        {
            result |= (~0ull << shift);
        }

        return result;
    }
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

std::string fileStructure::leb128string(int ptr)
{
    int ptrx = ptr;
    int len = leb128u(ptr);
    ptrx += leb128Size;
    std::string s = "";
    while (len > 0)
    {
        if ((raw[ptrx] >= 32) && (raw[ptrx] <= 126))
        {
            s.push_back(raw[ptrx]);
        }
        else
        {
            s.push_back('_');
        }
        ptrx++;
        len--;
    }
    leb128Size = ptrx - ptr;
    return s;
}
