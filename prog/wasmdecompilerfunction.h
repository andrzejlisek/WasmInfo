#ifndef WASMDECOMPILERFUNCTION_H
#define WASMDECOMPILERFUNCTION_H

#include <string>
#include <memory>
#include <vector>
#include <iostream>
#include "hex.h"

class wasmDecompilerFunction
{
private:
public:
    wasmDecompilerFunction();
    std::string name = "";
    std::string returnName = "";
    std::vector<std::string> returnNameItems;
    std::vector<std::shared_ptr<wasmDecompilerFunction>> params;
    int depth = 0;
    bool isFoldable = false;
    bool blockFold = false;
    std::vector<unsigned char> originalInstr;
    std::string instrText();
    void paramAdd(std::string text);
    void additionalInstr(int currentDepth, std::string instrResult, std::string instrParam, int instrId, std::string comment);
    std::string comment = "";
    int id = 0;
    bool printComma = false;
};

#endif // WASMDECOMPILERFUNCTION_H
