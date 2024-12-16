#ifndef FILESTRUCTURE_H
#define FILESTRUCTURE_H

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include "hex.h"
#include "wasmdecompiler.h"

typedef unsigned char uchar;
typedef unsigned int uint;
typedef int sint;

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
    };

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
        bool _CodeGood;
        std::vector<int> _CodeLocalN;
        std::vector<int> _CodeLocalType;
        std::vector<int> _CodeLocalAddr;
        std::vector<int> _CodeLocalSize;

        std::vector<codeInstr> _CodeInstr;
    };
    int parseFunctionId;
    int parseMemoId;
    int parseTableId;
    int parseGlobalId;
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
    void parseStart(sectionInfo &sectionInfo__);
    bool isCodeGood(sectionSubInfo &sectionSubInfo__);
    int parseInstruction(int ptr, sectionSubInfo &sectionSubInfo__);
    int parseInstructions(int addr, sectionSubInfo &sectionSubInfo__);
    std::string instructionText(codeInstr codeInstr_, int fidx);
    std::string itemInfoText(sectionSubInfo &sectionSubInfo__);
public:
    fileStructure();
    void parse(uchar * raw_, int rawSize_);
    void printType(std::stringstream &ss, sectionInfo &sectionInfo__, bool infoItem, bool infoItemRaw, bool infoCode);
    void printImport(std::stringstream &ss, sectionInfo &sectionInfo__, bool infoItem, bool infoItemRaw, bool infoCode);
    void printFunction(std::stringstream &ss, sectionInfo &sectionInfo__, bool infoItem, bool infoItemRaw, bool infoCode);
    void printTable(std::stringstream &ss, sectionInfo &sectionInfo__, bool infoItem, bool infoItemRaw, bool infoCode);
    void printMemory(std::stringstream &ss, sectionInfo &sectionInfo__, bool infoItem, bool infoItemRaw, bool infoCode);
    void printGlobal(std::stringstream &ss, sectionInfo &sectionInfo__, bool infoItem, bool infoItemRaw, bool infoCode);
    void printExport(std::stringstream &ss, sectionInfo &sectionInfo__, bool infoItem, bool infoItemRaw, bool infoCode);
    void printStart(std::stringstream &ss, sectionInfo &sectionInfo__, bool infoItem, bool infoItemRaw, bool infoCode);
    void printElement(std::stringstream &ss, sectionInfo &sectionInfo__, bool infoItem, bool infoItemRaw, bool infoCode);
    void printCode(std::stringstream &ss, sectionInfo &sectionInfo__, bool infoItem, bool infoItemRaw, bool infoCode);
    void printData(std::stringstream &ss, sectionInfo &sectionInfo__, bool infoItem, bool infoItemRaw, bool infoCode);
    void printDataCount(std::stringstream &ss, sectionInfo &sectionInfo__, bool infoItem, bool infoItemRaw, bool infoCode);
    void printRaw(std::stringstream &ss, int rawAddr, int rawSize);
    std::string print(int codeBinSize_, int sectionId, bool infoRaw, bool infoItem, bool infoItemRaw, bool infoCode);
    void printCodeText(std::stringstream &ss, int addr, int count);
    void printCodeInstr(std::stringstream &ss, std::vector<codeInstr> &codeInstr_, int fidx);

    void loadNames(int setType, std::string nameText);
private:
    std::string correctFunctionName(std::string funcName);
    int getVarTypeG(int idx);
    int getVarTypeL(int idx, int fidx_);
    std::string getFunctionType(int funcType, std::string funcName, int &localNum);
    std::string getFunctionName(int idx, int fidx, int &localNum);
    std::string getFunctionNameById(int idx, int fidx, int localNum);

    uchar * raw;
    int rawSize;

    std::vector<sectionInfo> sectionInfo_;
    std::vector<sectionSubInfo> sectionSubInfo_;

    int leb128Size;
    uint leb128u(int ptr);
    sint leb128s(int ptr, int size);
    uint intu(int ptr);
    sint ints(int ptr);
    std::string f32tostr(int ptr);
    std::string f64tostr(int ptr);
};

#endif // FILESTRUCTURE_H
