#ifndef FILESTRUCTURE_H
#define FILESTRUCTURE_H

#include <iostream>
#include <string>
#include <vector>
#include "hex.h"
#include "wasmdecompiler.h"
#include "stringbuf.h"

typedef unsigned char uchar;
typedef unsigned int uint;
typedef signed int sint;
typedef unsigned long long ullong;
typedef signed long long sllong;

class fileStructure
{
public:
    int codeBinSize = 10;
private:
    wasmDecompiler wasmDecompiler_;
    struct sectionInfo
    {
        int ParseStatus;
        int StartIdx;
        int Addr;
        int Size;
        int Type;
        int Id;
        int DataAddr;
        int DataSize;
        int SubCount;
        int SubAddr;
        int SubIdx1;
        int SubIdx2;
    };
    struct codeInstr
    {
        int Addr;
        int Size;
        int Opcode;
        std::string Param0;
        std::string Param1;
        std::string Param2;
        int Depth;
        int stackP;
        int stackR;
        int stackS;
        std::string stackR_;
        std::string stackI_;
        std::string stackO_;
        std::string errorMsg;
    };

    std::string codeInstrInfoBlank();
    std::string codeInstrInfo(codeInstr codeInstr_);

    struct sectionSubInfo
    {
        int ItemAddr;
        int ItemSize;
        int Addr;
        int Index;
        std::vector<int> _TypeParams;
        std::vector<int> _TypeReturn;
        int _FunctionIdx;
        std::string _FunctionName;
        int _FunctionTag;
        int _CodeAddr;
        int _CodeAddr_;
        int _CodeSize;
        bool _CodeGood = false;
        std::vector<int> _CodeLocalN;
        std::vector<int> _CodeLocalType;
        std::vector<int> _CodeLocalAddr;
        std::vector<int> _CodeLocalSize;

        std::vector<codeInstr> _CodeInstr;

        std::vector<wasmDecompiler::dataField2> _CodeDataFieldDictionary;
    };
    int parseFunctionId;
    int parseMemoId;
    int parseTableId;
    int parseGlobalId;
    void parseCustom(sectionInfo &sectionInfo__, int idx);
    void parseType(sectionInfo &sectionInfo__);
    void parseFunction(sectionInfo &sectionInfo__);
    void parseExport(sectionInfo &sectionInfo__);
    void parseImport(sectionInfo &sectionInfo__);
    void parseGlobal(sectionInfo &sectionInfo__);
    void parseData(sectionInfo &sectionInfo__);
    void parseDataCount(sectionInfo &sectionInfo__);
    void parseTable(sectionInfo &sectionInfo__);
    void parseMemory(sectionInfo &sectionInfo__);
    void parseElement(sectionInfo &sectionInfo__);
    void parseCode(sectionInfo &sectionInfo__);
    void parseCode2(sectionInfo &sectionInfo__);
    void parseStart(sectionInfo &sectionInfo__);
    void parseTag(sectionInfo &sectionInfo__);
    bool isCodeGood(sectionSubInfo &sectionSubInfo__);
    std::string valueTypeNameEx(int valueType);
    int parseInstruction(int ptr, sectionSubInfo &sectionSubInfo__);
    int parseInstructions(int addr, sectionSubInfo &sectionSubInfo__, int sectionSubInfoI, int returnTypeX);
    std::string instructionText(codeInstr codeInstr_, int fidx);
    std::string itemInfoText(int sectionId, sectionSubInfo &sectionSubInfo__);
    std::string sizeText(int minVal, int maxVal);
public:
    fileStructure();
    void parse(uchar * raw_, int rawSize_);
    void printCustom(stringBuf &sb, sectionInfo &sectionInfo__, bool infoItem, bool infoItemRaw, bool infoCode, int itemIndex);
    void printType(stringBuf &sb, sectionInfo &sectionInfo__, bool infoItem, bool infoItemRaw, bool infoCode, int itemIndex);
    void printImport(stringBuf &sb, sectionInfo &sectionInfo__, bool infoItem, bool infoItemRaw, bool infoCode, int itemIndex);
    void printFunction(stringBuf &sb, sectionInfo &sectionInfo__, bool infoItem, bool infoItemRaw, bool infoCode, int itemIndex);
    void printTable(stringBuf &sb, sectionInfo &sectionInfo__, bool infoItem, bool infoItemRaw, bool infoCode, int itemIndex);
    void printMemory(stringBuf &sb, sectionInfo &sectionInfo__, bool infoItem, bool infoItemRaw, bool infoCode, int itemIndex);
    void printGlobal(stringBuf &sb, sectionInfo &sectionInfo__, bool infoItem, bool infoItemRaw, bool infoCode, int decompType, int decompBranch, int itemIndex);
    void printExport(stringBuf &sb, sectionInfo &sectionInfo__, bool infoItem, bool infoItemRaw, bool infoCode, int itemIndex);
    void printStart(stringBuf &sb, sectionInfo &sectionInfo__, bool infoItem, bool infoItemRaw, bool infoCode);
    void printElement(stringBuf &sb, sectionInfo &sectionInfo__, bool infoItem, bool infoItemRaw, bool infoCode, int decompType, int decompBranch, int itemIndex);
    void printCode(stringBuf &sb, sectionInfo &sectionInfo__, bool infoItem, bool infoItemRaw, bool infoCode, int decompType, int decompBranch, int itemIndex);
    void printData(stringBuf &sb, sectionInfo &sectionInfo__, bool infoItem, bool infoItemRaw, bool infoCode, int decompType, int decompBranch, int itemIndex);
    void printDataCount(stringBuf &sb, sectionInfo &sectionInfo__, bool infoItem, bool infoItemRaw, bool infoCode);
    void printTag(stringBuf &sb, sectionInfo &sectionInfo__, bool infoItem, bool infoItemRaw, bool infoCode, int itemIndex);
    void printRaw(stringBuf &sb, int rawAddr, int rawSize);
    std::string print(int codeBinSize_, int sectionId, bool infoRaw, bool infoItem, bool infoItemRaw, bool infoCode, int decompType, int decompBranch, int decompOpts, int itemIndex);
    void printCodeText(stringBuf &sb, int addr, int count);
    void printCodeInstr(stringBuf &sb, std::vector<codeInstr> &codeInstr_, int fidx, int decompType);

    void loadNames(int setType, std::string nameText);

private:
    int getTableType(int idx);
    int getVarTypeG(int idx);
    int getVarTypeL(int idx, int fidx_);
    int getVarTypeL(int idx, int fidx_, sectionSubInfo &sectionSubInfo__);
    std::string getGlobalVarName(int num, bool def, int valType, bool rawname);
    int getTypeListItemByTag(int num);

    enum getFunctionNameByIdMode { debug, funcNameString, funcTypeNumber, funcCode };
    enum getFunctionNameByIdNumber { whole, internal, type, import };
    std::string getFunctionNameById(getFunctionNameByIdMode mode, getFunctionNameByIdNumber number, int idx);
    std::string getFunctionNameByIdLocalNum(getFunctionNameByIdMode mode, getFunctionNameByIdNumber number, int idx, int &localNum);

public: //!!!!!!!!!!11
    uchar * raw;
    int rawSize;

    std::vector<sectionInfo> sectionInfo_;
    std::vector<sectionSubInfo> sectionSubInfo_;

    int leb128Size;
    uint leb128u(int ptr);
    sllong leb128s(int ptr, int size);
    uint intu(int ptr);
    sint ints(int ptr);
    std::string f32tostr(int ptr);
    std::string f64tostr(int ptr);
    std::string leb128string(int ptr);
};

#endif // FILESTRUCTURE_H
