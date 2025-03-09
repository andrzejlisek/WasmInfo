#ifndef HEX_H
#define HEX_H

#include <string>
#include <sstream>
#include <iomanip>

class hex
{
public:
    hex();
    static std::string IntToHex(int Data);
    static std::string IntToHex4(int Data);
    static std::string IntToHex8(int Data);
    static std::string IntToHex16(int Data);
    static std::string IntToHex32(int Data);
    static std::string IntToHex32(unsigned int Data);

    static std::string ToHex(char * Raw, unsigned int RawSize);
    static std::string ToHex(unsigned char * Raw, unsigned int RawSize);

    static int HexToInt(std::string Hex0);
    static int HexToInt(char Hex0);

    static std::string StringGetParam(std::string S, int N, char Delimiter);
    static int StringIndexOf(std::string S, std::string Substr);
    static std::string StringFindReplace(std::string S, std::string From, std::string To);
    static std::string indent(int n);
private:
    static std::string floatToStr_(std::string NumStr, int Digits);
public:
    static std::string floatToStr(double Num, int Digits);
    static std::string singleToStr(double Num);
    static std::string doubleToStr(double Num);
};

#endif // HEX_H
