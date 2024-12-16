#ifndef WASMDECOMPILER_H
#define WASMDECOMPILER_H

#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include "hex.h"
#include "wasmdecompilerfunction.h"

class wasmDecompiler
{
private:
    bool useSimplified = true;
    bool debugInfo = false;
    int debugOptimizeIterations = 1000000000;

    struct dataField
    {
        std::string name;
        int type;
    };

    std::vector<dataField> dataGlobal;
    std::vector<dataField> dataParam;
    std::vector<dataField> dataLocal;
    std::vector<dataField> dataReturn;

    std::string debugValues(int idx);

    wasmDecompilerFunction WDF;
    int currentDepth;
    int currentStack;

    void codeOptimize();

    int lastOpcode;
    std::vector<int> stackSizeBlock;
    int tempVarCounter;
    std::string resultVarPrefix = "temp";

public:
    wasmDecompiler();

    struct codeDef
    {
        std::string paramAsm;
        std::string nameAsm;
        std::string nameDecomp;
        std::string stackParam;
        std::string stackResult;
    };

    codeDef codeDef_[6][256];

    std::string funcName;
    void reset(std::string funcName_);
    void addCommand(unsigned char * raw, int addr, int size, std::string par0, std::string par1, std::string par2);
    void addGlobal(int idx, int dataType);
    void addParam(int idx, int dataType);
    void addLocal(int idx, int dataType);
    void addReturn(int idx, int dataType);
    std::string printCommand(int idx);
    std::string valueTypeName(int typeSig);
};

#endif // WASMDECOMPILER_H
