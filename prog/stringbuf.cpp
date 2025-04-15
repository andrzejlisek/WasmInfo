#include "stringbuf.h"

stringBuf::stringBuf()
{
    eol_ = "\n";
    clear(false);
}

stringBuf::~stringBuf()
{
    clear(false);
}

stringBuf &stringBuf::append(std::string str)
{
    if (useHtml)
    {
        int l = str.size();
        bool htmlExists = false;
        for (int i = 0; i < l; i++)
        {
            switch (str[i])
            {
                case 0x1B:
                case '<':
                case '>':
                case '&':
                case '\"':
                case '\'':
                    htmlExists = true;
                    i = l;
                    break;
            }
        }

        if (htmlExists)
        {
            std::string str0;
            for (int i = 0; i < l; i++)
            {
                char c = str[i];
                if (htmlConvRaw)
                {
                    switch (c)
                    {
                        case '\n':
                            htmlConvRaw = false;
                            str0.push_back(c);
                            break;
                        case 0x1B:
                            htmlConvRaw = false;
                            break;
                        default:
                            str0.push_back(c);
                            break;
                    }
                }
                else
                {
                    switch (c)
                    {
                        case '<':
                            str0.push_back('&');
                            str0.push_back('l');
                            str0.push_back('t');
                            str0.push_back(';');
                            break;
                        case '>':
                            str0.push_back('&');
                            str0.push_back('g');
                            str0.push_back('t');
                            str0.push_back(';');
                            break;
                        case '&':
                            str0.push_back('&');
                            str0.push_back('a');
                            str0.push_back('m');
                            str0.push_back('p');
                            str0.push_back(';');
                            break;
                        case '\"':
                            str0.push_back('&');
                            str0.push_back('q');
                            str0.push_back('u');
                            str0.push_back('o');
                            str0.push_back('t');
                            str0.push_back(';');
                            break;
                        case '\'':
                            str0.push_back('&');
                            str0.push_back('a');
                            str0.push_back('p');
                            str0.push_back('o');
                            str0.push_back('s');
                            str0.push_back(';');
                            break;
                        default:
                            str0.push_back(c);
                            break;
                        case 0x1B:
                            htmlConvRaw = true;
                            break;
                    }
                }
            }
            str = str0;
        }
    }

    length_ += str.size();
    unflushed += str.size();
    buf << str;
    return *this;
}

stringBuf &stringBuf::append(int num)
{
    append(std::to_string(num));
    return *this;
}

stringBuf &stringBuf::eol()
{
    append(eol_);
    if (unflushed >= 1000000)
    {
        flush();
    }
    return *this;
}

void stringBuf::clear(bool useHtml_)
{
    htmlConvRaw = false;
    useHtml = useHtml_;
    length_ = 0;
    unflushed = 0;
    buf.str("");
    bufVec.clear();
}

void stringBuf::flush()
{
    if (unflushed > 0)
    {
        bufVec.push_back(buf.str());
        buf.str("");
    }
    unflushed = 0;
}

int stringBuf::length()
{
    return length_;
}

std::string stringBuf::getString()
{
    flush();
    std::stringstream ss;
    for (int i = 0; i < bufVec.size(); i++)
    {
        ss << bufVec[i];
    }
    clear(useHtml);
    return ss.str();
}
