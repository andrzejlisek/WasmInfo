#include "hex.h"

hex::hex()
{

}

std::string hex::IntToHex(int Data)
{
    if (Data == 0)
    {
        return "0";
    }
    std::string H = IntToHex32(Data);
    while (H[0] == '0')
    {
        H = H.substr(1);
    }
    return H;
}

std::string hex::IntToHex4(int Data)
{
    switch (Data)
    {
        case 0: return "0";
        case 1: return "1";
        case 2: return "2";
        case 3: return "3";
        case 4: return "4";
        case 5: return "5";
        case 6: return "6";
        case 7: return "7";
        case 8: return "8";
        case 9: return "9";
        case 10: return "A";
        case 11: return "B";
        case 12: return "C";
        case 13: return "D";
        case 14: return "E";
        case 15: return "F";
    }
    return "?";
}

std::string hex::IntToHex8(int Data)
{
    int D1 = Data >> 4;
    int D2 = Data & 15;
    return IntToHex4(D1) + IntToHex4(D2);
}

std::string hex::IntToHex16(int Data)
{
    int D1 = Data >> 8;
    int D2 = Data & 255;
    return IntToHex8(D1) + IntToHex8(D2);
}

std::string hex::IntToHex32(int Data)
{
    int D1 = Data >> 16;
    int D2 = Data & 65535;
    return IntToHex16(D1) + IntToHex16(D2);
}

std::string hex::IntToHex32(unsigned int Data)
{
    unsigned int D1 = Data >> 16;
    unsigned int D2 = Data & 65535;
    return IntToHex16((int)D1) + IntToHex16((int)D2);
}

std::string hex::ToHex(char *Raw, unsigned int RawSize)
{
    return ToHex((unsigned char*)Raw, RawSize);
}

std::string hex::ToHex(unsigned char *Raw, unsigned int RawSize)
{
    std::string X = "";
    for (unsigned int I = 0; I < RawSize; I++)
    {
        X = X + hex::IntToHex8(Raw[I]);
    }
    return X;
}

int hex::HexToInt(std::string Hex0)
{
    int L = Hex0.length();
    std::string Hex = "";
    for (int i = 0; i < L; i++)
    {
        Hex = Hex0[i] + Hex;
    }
    int D = 0;
    int N = 0;
    for (int i = L - 1; i >= 0; i--)
    {
        D = 0;
        if (Hex[i] == '0') { D = 0; }
        if (Hex[i] == '1') { D = 1; }
        if (Hex[i] == '2') { D = 2; }
        if (Hex[i] == '3') { D = 3; }
        if (Hex[i] == '4') { D = 4; }
        if (Hex[i] == '5') { D = 5; }
        if (Hex[i] == '6') { D = 6; }
        if (Hex[i] == '7') { D = 7; }
        if (Hex[i] == '8') { D = 8; }
        if (Hex[i] == '9') { D = 9; }
        if (Hex[i] == 'A') { D = 10; }
        if (Hex[i] == 'B') { D = 11; }
        if (Hex[i] == 'C') { D = 12; }
        if (Hex[i] == 'D') { D = 13; }
        if (Hex[i] == 'E') { D = 14; }
        if (Hex[i] == 'F') { D = 15; }
        if (Hex[i] == 'a') { D = 10; }
        if (Hex[i] == 'b') { D = 11; }
        if (Hex[i] == 'c') { D = 12; }
        if (Hex[i] == 'd') { D = 13; }
        if (Hex[i] == 'e') { D = 14; }
        if (Hex[i] == 'f') { D = 15; }
        if (i == 0) { N = N + (D); }
        if (i == 1) { N = N + (D * 16); }
        if (i == 2) { N = N + (D * 256); }
        if (i == 3) { N = N + (D * 4096); }
        if (i == 4) { N = N + (D * 65536); }
        if (i == 5) { N = N + (D * 1048576); }
        if (i == 6) { N = N + (D * 16777216); }
        if (i == 7) { N = N + (D * 268435456); }
    }
    return N;
}

int hex::HexToInt(char Hex0)
{
    switch (Hex0)
    {
        case '0': return 0;
        case '1': return 1;
        case '2': return 2;
        case '3': return 3;
        case '4': return 4;
        case '5': return 5;
        case '6': return 6;
        case '7': return 7;
        case '8': return 8;
        case '9': return 9;
        case 'A': return 10;
        case 'B': return 11;
        case 'C': return 12;
        case 'D': return 13;
        case 'E': return 14;
        case 'F': return 15;
        case 'a': return 10;
        case 'b': return 11;
        case 'c': return 12;
        case 'd': return 13;
        case 'e': return 14;
        case 'f': return 15;
    }
    return 0;
}

std::string hex::StringGetParam(std::string S, int N, char Delimiter)
{
    int C = 0;

    if (N < 0)
    {
        for (int i = 0; i < S.size(); i++)
        {
            if (S[i] == Delimiter) C++;
        }
        return std::to_string(C);
    }

    int i0 = 0;
    for (int i = 0; i < S.size(); i++)
    {
        if (S[i] == Delimiter)
        {
            if (C == (N - 1))
            {
                i0 = i + 1;
            }
            if (C == N)
            {
                return S.substr(i0, i - i0);
            }
            C++;
        }
    }

    return "!";
}

int hex::StringIndexOf(std::string S, std::string Substr)
{
    int I = S.find(Substr);
    return I;
}

std::string hex::StringFindReplaceFirst(std::string S, std::string From, std::string To)
{
    if (From == To)
    {
        return S;
    }

    if (S == From)
    {
        return To;
    }
    int FromLen = From.length();
    int FromPos = S.find(From);
    if (FromPos != std::string::npos)
    {
        S.replace(FromPos, FromLen, To);
        FromPos = S.find(From);
    }
    return S;
}

std::string hex::StringFindReplace(std::string S, std::string From, std::string To)
{
    if (From == To)
    {
        return S;
    }

    if (S == From)
    {
        return To;
    }
    int FromLen = From.length();
    int FromPos = S.find(From);
    while (FromPos != std::string::npos)
    {
        S.replace(FromPos, FromLen, To);
        FromPos = S.find(From);
    }
    return S;
}

std::string hex::indent(int n)
{
    std::string S = "";
    while (n > 0)
    {
        S.push_back(' ');
        S.push_back(' ');
        n--;
    }
    return S;
}

std::string hex::pad(int n)
{
    std::string S = "";
    while (n > 0)
    {
        S.push_back(' ');
        n--;
    }
    return S;
}

std::string hex::floatToStr_(std::string NumStr, int Digits)
{
    int I;
    int L = NumStr.size();
    int NPos1 = -1;
    int NPos2 = -1;
    int DecimalPos = -1;

    for (I = 0; I < L; I++)
    {
        switch (NumStr[I])
        {
            case '0':
                if (NPos1 >= 0)
                {
                    Digits--;
                    if ((Digits == 1) && (NPos2 < 0))
                    {
                        NPos2 = I;
                    }
                }
                break;
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                if (NPos1 >= 0)
                {
                    Digits--;
                    if ((Digits == 1) && (NPos2 < 0))
                    {
                        NPos2 = I;
                    }
                }
                else NPos1 = I;
                break;
            case '.':
                DecimalPos = I;
                break;
            default:
                return NumStr;
                break;
        }
    }

    if (NPos1 < 0) return "0";
    if (NPos2 < 0)
    {
        NPos2 = L - 1;
    }

    bool RoundUp = false;
    if ((NPos2 + 1) < L)
    {
        if (NumStr[NPos2 + 1] != '.')
        {
            if (NumStr[NPos2 + 1] >= '5')
            {
                RoundUp = true;
            }
        }
        else
        {
            if ((NPos2 + 2) < L)
            {
                if (NumStr[NPos2 + 2] >= '5')
                {
                    RoundUp = true;
                }
            }
        }
    }

    if (RoundUp)
    {
        bool AdditionalZero = false;
        NPos1--;
        if (NPos1 < 0)
        {
            AdditionalZero = true;
            NumStr = "0" + NumStr;
            NPos1++;
            NPos2++;
        }
        bool Carry = true;
        for (I = NPos2; I >= NPos1; I--)
        {
            if (NumStr[I] != '.')
            {
                if (Carry)
                {
                    if (NumStr[I] == '9')
                    {
                        NumStr[I] = '0';
                        Carry = true;
                    }
                    else
                    {
                        NumStr[I]++;
                        Carry = false;
                    }
                }
            }
        }

        if (AdditionalZero)
        {
            if (NumStr[0] == '0')
            {
                NumStr = NumStr.substr(1);
            }
            NPos2--;
        }
    }

    // Truncate garbage digits and fractional part
    if (NPos2 < DecimalPos)
    {
        while (NPos2 < DecimalPos)
        {
            NPos2++;
            NumStr[NPos2] = '0';
        }
        return NumStr.substr(0, DecimalPos);
    }

    // Truncate garbage digits
    if (NPos2 < (L - 1))
    {
        NumStr = NumStr.substr(0, NPos2 + 1);
    }

    // Truncate trailing zeros
    if (DecimalPos < NumStr.size())
    {
        I = NumStr.size() - 1;
        int I0 = I;
        while ((I > 1) && (NumStr[I] == '0'))
        {
            I--;
        }
        if (I < I0)
        {
            if (NumStr[I] == '.')
            {
                NumStr = NumStr.substr(0, I);
            }
            else
            {
                NumStr = NumStr.substr(0, I + 1);
            }
        }
    }

    return NumStr;
}


std::string hex::floatToStr(double Num, int Digits)
{
    bool NumNegative = false;
    if (Num < 0)
    {
        NumNegative = true;
        Num = 0 - Num;
    }
    double NumX = Num;
    int Decimals = Digits + 3;
    while ((NumX < 1) && (NumX > 0))
    {
        NumX = NumX * 10;
        Decimals++;
    }
    std::stringstream Buf;
    Buf << std::fixed << std::setprecision(Decimals) << std::noshowpoint << Num;
    std::string Buf_ = floatToStr_(Buf.str(), Digits);
    if (((int)Buf_.find(".")) < 0)
    {
        Buf_ = Buf_ + ".0";
    }
    if (NumNegative)
    {
        return "-" + Buf_;
    }
    else
    {
        return Buf_;
    }
}

std::string hex::singleToStr(double Num)
{
    return floatToStr(Num, 7);
}

std::string hex::doubleToStr(double Num)
{
    return floatToStr(Num, 15);
}
