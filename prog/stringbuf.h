#ifndef STRINGBUF_H
#define STRINGBUF_H

#include <sstream>
#include <vector>
#include <iostream>

class stringBuf
{
public:
    stringBuf();
    ~stringBuf();
    stringBuf &append(std::string str);
    stringBuf &append(int num);
    stringBuf &eol();
    void flush();
    void clear(bool useHtml_);
    std::string getString();
    int length();
private:
    int length_;
    std::stringstream buf;
    bool useHtml;
    std::string eol_;
    bool htmlConvRaw;
    std::vector<std::string> bufVec;
    int unflushed;
};

#endif // STRINGBUF_H
