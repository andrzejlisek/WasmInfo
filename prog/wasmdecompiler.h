#ifndef WASMDECOMPILER_H
#define WASMDECOMPILER_H

#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include "hex.h"
#include "wasmdecompilerfunction.h"
#include "wasmdecompilerindentdata.h"

class wasmDecompiler
{
public:
    bool useHtml = true;
    bool useTags = true;

    constexpr static int fieldType_i32 = 0x7F;
    constexpr static int fieldType_i64 = 0x7E;
    constexpr static int fieldType_f32 = 0x7D;
    constexpr static int fieldType_f64 = 0x7C;
    constexpr static int fieldType_v128 = 0x7B;
    constexpr static int fieldType_u_ = 0x01;
    constexpr static int fieldType_void = 0x40;
    constexpr static int fieldType_anyref = 0x6E;
    constexpr static int fieldType_funcref = 0x70;
    constexpr static int fieldType_externref = 0x6F;
    constexpr static int fieldType_exnref = 0x69;

    constexpr static int fieldTypeIgnore = 255;
    constexpr static int fieldTypeVariable = 255;
    constexpr static int fieldTypeCallFunc = 254;

    constexpr static int branchBlock = 1;
    constexpr static int branchLoop = 3;
    constexpr static int branchEnd = 4;

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

    void metaTagClear();
    void metaTagRemoveLast(int n);
    void metaTagAdd(int section, int type, int idx, int idxx, std::string name);
    std::string metaTagGetInfo(int section, int vecIdx);
    bool metaTagValid;
    int metaTagFunctionNumber = 0;

    std::string metaTagGetTempl(int type, int idx, std::string namePre, std::string nameSuf);
    std::string metaTagGet(int type, int idx, std::string name);
    std::string metaTagGet2(int type, int idx, int idxx, std::string name);
    void metaTagValidateNames();

    std::unordered_map<std::string, std::string> metaTagCache;

    struct metaTagDef
    {
        int section;
        int type;
        int idx;
        int idxx;
        std::string name;
        std::string sysname;
    };
    std::vector<metaTagDef> metaTag;

    std::string correctFunctionName(std::string funcName);

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

    std::string debugValues();

    wasmDecompilerFunction WDF;
    int currentDepth;
    int currentStack;
    std::vector<int> currentStack_;
    std::vector<int> currentStackStackP;
    std::vector<int> currentStackStackR;
    std::vector<std::string> currentStackStackP_;
    std::vector<std::string> currentStackStackR_;

    void convertBlockToLabels();
    void codeOptimize();

    std::vector<std::string> localVarNamesT;
    std::vector<int> localVarNamesD;

    int lastOpcode;
    std::vector<wasmDecompilerIndentData> indentStack;
    std::vector<int> valueTypeXNumber;
    std::vector<std::string> valueTypeXName;

public:

    bool instrBlockBegin(std::string txt);
    bool instrBlockEnd(std::string txt);

    wasmDecompiler();

    int codeInstrInfoLength = 0;
    int codeInstrInfoLengthDebug = 5;

    bool codeInstrInfoStack = true;

    void currentStackIgnore();
    void currentStackPush(int valType);
    bool currentStackPop();
    int currentStackSize(int sizeType);
    std::string currentStackPrint();
    bool currentStackBlockPush(int stackP, int stackR, std::string stackP_, std::string stackR_);
    bool currentStackBlockPop();
    void currentStackBlockPrepare(int stackP, int stackR, std::string stackP_, std::string stackR_);
    void currentStackBlockRestore(bool clearParams);
    std::string stackPrintInfo(std::string stackInfo, int stackDepth);
    std::string stackPrintInfoFull(int stackSize, std::string stackInfoI, int stackDepthI, std::string stackInfoO, int stackDepthO);
    std::string stackPrintInfoBlank();

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
    int addCommandBloop;
    void addCommand(unsigned char * raw, int addr, int size, std::string par0, std::string par1, std::string par2, std::string stackI__, std::string stackO__, int stackP__, int stackR__, int stackS__);
    void addCommand(std::string instr, std::string par0, std::string par1, std::string par2, std::string stackI__, std::string stackO__, int stackP__, int stackR__, int stackS__);
    void addCommandStackDummy(int valueType);
    std::vector<unsigned char> dummyRaw;
    std::string dataFieldDictionaryGetVar(std::string category, int num);
    std::string dataFieldDictionaryGetConst(int type, std::string val);
    int dataFieldDictionaryGetType(std::string category, int num);
    void dataFieldDictionarySetType(std::string dataIdX, int dataTypeX);
    std::string dataFieldDictionarySet(std::string dataNameX, int dataTypeX, std::string category, int num);
    int dataFieldDictionaryIdx(std::string id);
    std::string dataFieldDictionaryDisplay(std::string id);
    std::string printCommand(int idx);
    std::string valueTypeName(int typeSig);
    int valueTypeNumber(std::string typeSig);
    bool valueTypeIsStandard(int typeSig);

    std::string htmlPrefix(int id, int idx);
    std::string htmlSuffix(int id, int idx);
private:
    std::string dataName(int idx, int t);
};

#endif // WASMDECOMPILER_H
