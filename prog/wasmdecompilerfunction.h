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
    int stackI = 0;
    int stackP = 0;
    int stackR = 0;
    int stackO = 0;
    bool stackPrint = false;
    wasmDecompilerFunction();
    std::string name = "";
    std::string returnName = "";
    int returnType = -1;
    std::vector<int> returnTypeList;
    std::vector<std::string> returnNameItems;
    std::vector<std::shared_ptr<wasmDecompilerFunction>> params;
    int depth = 0;
    bool isFoldable = false;
    bool blockFold = false;
    std::vector<unsigned char> originalInstr;
    std::string instrText();
    int instrTextParamList = 0;
    void paramAdd(std::string text);
    void additionalInstr(int currentDepth, std::string instrResult, std::string instrParam, int instrId);
    int id = 0;
    bool printComma = false;
    int branchDepth = 0;
    int branchType = 0;
};

#endif // WASMDECOMPILERFUNCTION_H
