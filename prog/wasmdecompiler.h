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
public:
    constexpr static int fieldType_i32 = 0x7F;
    constexpr static int fieldType_i64 = 0x7E;
    constexpr static int fieldType_f32 = 0x7D;
    constexpr static int fieldType_f64 = 0x7C;
    constexpr static int fieldType_v128 = 0x7B;
    constexpr static int fieldType_u = 0x01;

    constexpr static int fieldTypeVariable = 255;
    constexpr static int fieldTypeCallFunc = 254;

    struct dataField2
    {
        std::string fieldCategory;
        std::string fieldId;
        std::string fieldName;
        int fieldType;
        int fieldNumber;
        bool isParam;
    };

    std::vector<dataField2> dataFieldDictionary;

    bool decompOptFold = true;
    bool decompOptStackSimplify = true;
    bool decompOptVariableDeclare = true;
    bool decompOptVariableHungarian = true;

private:
    bool debugInfo = false;
    bool debugNoLabelOptimize = false;
    bool debugPrintVariableList = false;
    bool debugRawVariableNames = false;

    int decompBranch = 0;
    int branchDepthMagicNum = 9999;

    bool debugIsFunc(std::string funcName);

    struct dataField
    {
        std::string name;
        int type;
    };

    std::string dataFieldPrintName(dataField N)
    {
        if (N.type >= 256)
        {
            return "c" + valueTypeName(N.type - 256) + "_" + N.name;
        }
        else
        {
            return valueTypeName(N.type) + "_" + N.name;
        }
    }

    std::string debugValues();

    wasmDecompilerFunction WDF;
    int currentDepth;
    int currentStack;

    void convertBlockToLabels();
    void codeOptimize();

    std::vector<std::string> localVarNamesT;
    std::vector<int> localVarNamesD;

    int lastOpcode;
    std::vector<int> stackSizeBlock;
    std::vector<int> stackSizeIf;
    std::vector<int> stackSizeDiff;
    int tempVarCounterP;
    int tempVarCounterR;

public:
    wasmDecompiler();

    struct codeDef
    {
        std::string paramAsm;
        std::string nameAsm;
        std::string nameDecomp;
        std::string stackParam;
        std::string stackResult;
        int stackResultType;
    };

    codeDef codeDef_[6][256];

    std::string funcName;
    void reset(std::string funcName_, int decompBranch_, std::vector<dataField2> dataFieldDictionary_);
    void addCommand(unsigned char * raw, int addr, int size, std::string par0, std::string par1, std::string par2);
    void dataFieldDictionaryClear();
    std::string dataFieldDictionaryGetVar(std::string category, int num);
    std::string dataFieldDictionaryGetConst(int type, std::string val);
    int dataFieldDictionaryGetType(std::string category, int num);
    void dataFieldDictionarySetType(std::string dataIdX, int dataTypeX);
    std::string dataFieldDictionarySet(std::string dataNameX, int dataTypeX, std::string category, int num);
    int dataFieldDictionaryIdx(std::string id);
    std::string dataFieldDictionaryDisplay(std::string id);
    std::string printCommand(int idx);
    std::string valueTypeName(int typeSig);
private:
    std::string dataName(int idx, int t);
};

#endif // WASMDECOMPILER_H
