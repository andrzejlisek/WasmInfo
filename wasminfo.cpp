#include <emscripten.h>
#include "prog/filestructure.h"
#include <string>
#include <vector>

fileStructure fileStructure_;
std::vector<unsigned char> wasmRaw;
std::string wasmText;

extern "C"
{
    EMSCRIPTEN_KEEPALIVE
    void loadNames(int setType, char * nameText)
    {
        std::string nameText_(nameText);
        fileStructure_.loadNames(setType, nameText_);
    }


    EMSCRIPTEN_KEEPALIVE
    void loadBinary(char * contents)
    {
        wasmRaw.clear();

        char Base64Chars[66] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
        
        int L = 0;
        while (contents[L])
        {
            L++;
        }
        L = L >> 2;
        L = L << 2;
        for (int I = 0; I < L; I += 4)
        {
            int Byte0 = -1;
            int Byte1 = -1;
            int Byte2 = -1;
            int Byte3 = -1;
            for (int II = 0; II < 64; II++)
            {
                if (Base64Chars[II] == contents[I + 0]) { Byte0 = II; }
                if (Base64Chars[II] == contents[I + 1]) { Byte1 = II; }
                if (Base64Chars[II] == contents[I + 2]) { Byte2 = II; }
                if (Base64Chars[II] == contents[I + 3]) { Byte3 = II; }
            }

            if ((Byte0 >= 0) && (Byte1 >= 0))
            {
                wasmRaw.push_back(((Byte0 << 2) + (Byte1 >> 4)) & 255);
            }
            if ((Byte1 >= 0) && (Byte2 >= 0))
            {
                wasmRaw.push_back(((Byte1 << 4) + (Byte2 >> 2)) & 255);
            }
            if ((Byte2 >= 0) && (Byte3 >= 0))
            {
                wasmRaw.push_back(((Byte2 << 6) + (Byte3)) & 255);
            }
        }

        for (int i = 0; i < 8; i++)
        {
            wasmRaw.push_back(0);
        }

        fileStructure_.parse(wasmRaw.data(), wasmRaw.size() - 8);
    }

    EMSCRIPTEN_KEEPALIVE
    const char * getInfo(int codeBinSize, int sectionId, int infoRaw, int infoItem, int infoItemRaw, int infoCode, int decompType, int decompBranch)
    {
        wasmText = fileStructure_.print(codeBinSize, sectionId, infoRaw, infoItem, infoItemRaw, infoCode, decompType, decompBranch);
        return wasmText.c_str();
    }
}


int main()
{
    emscripten_run_script("initFinish();");
    return 0;
}

