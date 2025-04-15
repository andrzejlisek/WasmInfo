#include "filestructure.h"

std::string fileStructure::codeInstrInfoBlank()
{
    if (wasmDecompiler_.codeInstrInfoStack)
    {
        return wasmDecompiler_.stackPrintInfoBlank();
    }
    else
    {
        return "";
    }
}

std::string fileStructure::codeInstrInfo(codeInstr codeInstr_)
{
    if (codeInstr_.stackS < 0)
    {
        return codeInstrInfoBlank();
    }
    if (wasmDecompiler_.codeInstrInfoStack)
    {
        return wasmDecompiler_.stackPrintInfoFull(codeInstr_.stackS, codeInstr_.stackI_, codeInstr_.stackP, codeInstr_.stackO_, codeInstr_.stackR);
    }
    else
    {
        return "";
    }
}


void fileStructure::printCodeText(stringBuf &sb, int addr, int count)
{
    if (addr >= 0)
    {
        sb.append(hex::IntToHex32(addr)).append(":");
        if (count > codeBinSize)
        {
            count = codeBinSize;
        }
        for (int i = 0; i < count; i++)
        {
            sb.append(" ").append(hex::IntToHex8(raw[addr + i]));
        }
        for (int i = count; i < codeBinSize; i++)
        {
            sb.append("   ");
        }
    }
    else
    {
        sb.append("         ");
        for (int i = 0; i < codeBinSize; i++)
        {
            sb.append("   ");
        }
    }
    sb.append("  ");
}

void fileStructure::printCodeInstr(stringBuf &sb, std::vector<codeInstr> &codeInstr_, int fidx, int decompType)
{
    bool noCodeError = true;
    for (int iii = 0; iii < codeInstr_.size(); iii++)
    {
        if (!codeInstr_[iii].errorMsg.empty())
        {
            noCodeError = false;
        }
        if (codeInstr_[iii].Opcode < 0)
        {
            noCodeError = false;
        }
    }
    if (!noCodeError)
    {
        wasmDecompiler_.addCommandStackDummy(-2);
    }

    // Search for bloop
    wasmDecompiler_.addCommandBloop = 0;
    for (int iii = 0; iii < codeInstr_.size(); iii++)
    {
        switch (raw[codeInstr_[iii].Addr])
        {
            case 0x02: // block
            case 0x03: // loop
                if (wasmDecompiler_.addCommandBloop == 0) wasmDecompiler_.addCommandBloop = 1;
                break;
            case 0x0C: // br
            case 0x0D: // br_if
                wasmDecompiler_.addCommandBloop = 2;
                break;
        }
    }
    for (int iii = 0; iii < codeInstr_.size(); iii++)
    {
        switch (raw[codeInstr_[iii].Addr])
        {
            // The call functions requires additional type information
            case 0x10: // call
            case 0x11: // call_indirect
            case 0x12: // return_call
            case 0x13: // return_call_indirect
            case 0x14: // call_ref
            case 0x15: // return_call_ref
                {
                    int typeIdx = 0;
                    int dummy = -3;
                    switch (raw[codeInstr_[iii].Addr])
                    {
                        case 0x11: // call_indirect
                        case 0x13: // return_call_indirect
                            typeIdx = atoi(getFunctionNameById(getFunctionNameByIdMode::funcTypeNumber, getFunctionNameByIdNumber::type, atoi(codeInstr_[iii].Param0.c_str())).c_str());
                            break;
                        default:
                            typeIdx = atoi(getFunctionNameById(getFunctionNameByIdMode::funcTypeNumber, getFunctionNameByIdNumber::whole, atoi(codeInstr_[iii].Param0.c_str())).c_str());
                            break;
                    }

                    int stackP = sectionSubInfo_[typeIdx]._TypeParams.size();
                    int stackR = sectionSubInfo_[typeIdx]._TypeReturn.size();
                    std::string stackInfo = std::to_string(stackP) + "`" + std::to_string(stackR) + "`" + getFunctionNameById(getFunctionNameByIdMode::funcNameString, getFunctionNameByIdNumber::whole, atoi(codeInstr_[iii].Param0.c_str())) + "`";
                    for (int iiii = 0; iiii < sectionSubInfo_[typeIdx]._TypeReturn.size(); iiii++)
                    {
                        stackInfo = stackInfo + std::to_string(sectionSubInfo_[typeIdx]._TypeReturn[iiii]) + "`";
                    }
                    if (noCodeError) wasmDecompiler_.addCommand(raw, codeInstr_[iii].Addr, codeInstr_[iii].Size, codeInstr_[iii].Param0, codeInstr_[iii].Param1, stackInfo, codeInstr_[iii].stackI_, codeInstr_[iii].stackO_, codeInstr_[iii].stackP, codeInstr_[iii].stackR, codeInstr_[iii].stackS);
                }
                break;
            case 0x0F: // return
                {
                    if (noCodeError)
                    {
                        int returnCount = 0;
                        for (int i = 0; i < wasmDecompiler_.dataFieldDictionary.size(); i++)
                        {
                            if (wasmDecompiler_.dataFieldDictionary[i].fieldCategory == "return")
                            {
                                returnCount++;
                            }
                        }

                        if (returnCount > 0)
                        {
                            wasmDecompiler_.addCommand("0F", "", "", "", codeInstr_[iii].stackI_, codeInstr_[iii].stackI_ + "_void", returnCount, 0, codeInstr_[iii].stackS - returnCount);
                        }
                    }
                }
                break;
            case 0x0B: // end
                {
                    if (noCodeError)
                    {
                        if ((!codeInstr_[iii].stackO_.empty()) && (codeInstr_[iii].stackO_[0] != '|') && (((int)codeInstr_[iii].stackI_.find("void")) < 0))
                        {
                            int returnCount = 0;
                            for (int i = 0; i < wasmDecompiler_.dataFieldDictionary.size(); i++)
                            {
                                if (wasmDecompiler_.dataFieldDictionary[i].fieldCategory == "return")
                                {
                                    returnCount++;
                                }
                            }

                            if (returnCount > 0)
                            {
                                wasmDecompiler_.addCommand("0F", "", "", "", codeInstr_[iii].stackI_, codeInstr_[iii].stackI_ + "_void", returnCount, 0, codeInstr_[iii].stackS - returnCount);
                            }
                        }

                        wasmDecompiler_.addCommand(raw, codeInstr_[iii].Addr, codeInstr_[iii].Size, codeInstr_[iii].Param0, codeInstr_[iii].Param1, codeInstr_[iii].Param2, codeInstr_[iii].stackI_, codeInstr_[iii].stackO_, codeInstr_[iii].stackP, codeInstr_[iii].stackR, codeInstr_[iii].stackS);
                    }
                }
                break;
            default:
                {
                    if (noCodeError) wasmDecompiler_.addCommand(raw, codeInstr_[iii].Addr, codeInstr_[iii].Size, codeInstr_[iii].Param0, codeInstr_[iii].Param1, codeInstr_[iii].Param2, codeInstr_[iii].stackI_, codeInstr_[iii].stackO_, codeInstr_[iii].stackP, codeInstr_[iii].stackR, codeInstr_[iii].stackS);
                }
                break;
        }

        if (decompType & 1)
        {
            std::string instrText = instructionText(codeInstr_[iii], fidx);

            bool isBlockBegin = wasmDecompiler_.instrBlockBegin(instrText);
            bool isBlockEnd = wasmDecompiler_.instrBlockEnd(instrText);
            std::vector<int> instrTextLinePos;
            instrTextLinePos.push_back(-1);
            for (int i = 0; i < instrText.size(); i++)
            {
                if (instrText[i] == '\n')
                {
                    instrTextLinePos.push_back(i);
                }
            }
            instrTextLinePos.push_back(instrText.size());

            int stackPos = 1;
            if (isBlockBegin && isBlockEnd)
            {
                stackPos = instrTextLinePos.size() - 2;
                if (stackPos < 1) stackPos = 1;
            }
            else
            {
                if (isBlockBegin)
                {
                    stackPos = instrTextLinePos.size() - 1;
                    if (stackPos < 1) stackPos = 1;
                }
            }

            for (int i = 1; i < instrTextLinePos.size(); i++)
            {
                int idxPos = instrTextLinePos[i - 1] + 1;
                int idxLen = instrTextLinePos[i] - idxPos;

                printCodeText(sb, codeInstr_[iii].Addr, codeInstr_[iii].Size);

                if ((codeInstr_[iii].Depth > 0) && (i == stackPos))
                {
                    sb.append(codeInstrInfo(codeInstr_[iii]));
                }
                else
                {
                    sb.append(codeInstrInfoBlank());
                }

                sb.append(hex::indent(codeInstr_[iii].Depth)).append(instrText.substr(idxPos, idxLen)).append(codeInstr_[iii].errorMsg).eol();
            }
        }
    }
    if (decompType & 2)
    {
        int idx = 0;
        std::string decompInstr = wasmDecompiler_.printCommand(idx);
        while (decompInstr != "`")
        {
            int decompInstrLine = hex::StringIndexOf(decompInstr, "[\\n]");
            while (decompInstrLine > 0)
            {
                printCodeText(sb, -1, 0);
                sb.append(decompInstr.substr(0, decompInstrLine)).eol();
                decompInstr = decompInstr.substr(decompInstrLine + 4);
                decompInstrLine = hex::StringIndexOf(decompInstr, "[\\n]");
            }

            printCodeText(sb, -1, 0);
            sb.append(decompInstr).eol();
            idx++;
            decompInstr = wasmDecompiler_.printCommand(idx);
        }
    }
    else
    {
        std::string decompInstr = wasmDecompiler_.printCommand(0);
        if (decompInstr != "`")
        {
            printCodeText(sb, -1, 0);
            sb.append(decompInstr).eol();
        }
    }
}

int fileStructure::getVarTypeG(int idx)
{
    int t = 0;
    for (int i = 0; i < sectionInfo_.size(); i++)
    {
        if (sectionInfo_[i].Id == 2)
        {
            for (int ii = sectionInfo_[i].SubIdx1; ii < sectionInfo_[i].SubIdx2; ii++)
            {
                if (sectionSubInfo_[ii]._FunctionTag == 3)
                {
                    if (t == idx)
                    {
                        if (sectionSubInfo_[ii]._TypeReturn[1] == 0)
                        {
                            return 256 + sectionSubInfo_[ii]._TypeReturn[0];
                        }
                        else
                        {
                            return sectionSubInfo_[ii]._TypeReturn[0];
                        }
                    }
                    t++;
                }
            }
        }

        if (sectionInfo_[i].Id == 6)
        {
            for (int ii = sectionInfo_[i].SubIdx1; ii < sectionInfo_[i].SubIdx2; ii++)
            {
                if (t == idx)
                {
                    if (sectionSubInfo_[ii]._TypeReturn[1] == 0)
                    {
                        return 256 + sectionSubInfo_[ii]._TypeReturn[0];
                    }
                    else
                    {
                        return sectionSubInfo_[ii]._TypeReturn[0];
                    }
                }
                t++;
            }
        }
    }
    return 0;
}

int fileStructure::getVarTypeL(int idx, int fidx_)
{
    int fidx = fidx_ > 0 ? fidx_ : 0 - fidx_;
    int t = 0;
    int funcType = -1;
    for (int i = 0; i < sectionInfo_.size(); i++)
    {
        if (sectionInfo_[i].Id == 3)
        {
            for (int ii = sectionInfo_[i].SubIdx1; ii < sectionInfo_[i].SubIdx2; ii++)
            {
                if (sectionSubInfo_[ii].Index == sectionSubInfo_[fidx].Index)
                {
                    funcType = sectionSubInfo_[ii]._FunctionIdx;
                }
            }
        }
    }

    if (funcType >= 0)
    {
        for (int i = 0; i < sectionInfo_.size(); i++)
        {
            if (sectionInfo_[i].Id == 1)
            {
                for (int ii = sectionInfo_[i].SubIdx1; ii < sectionInfo_[i].SubIdx2; ii++)
                {
                    if (sectionSubInfo_[ii].Index == funcType)
                    {
                        if (fidx_ >= 0)
                        {
                            for (int iii = 0; iii < sectionSubInfo_[ii]._TypeParams.size(); iii++)
                            {
                                if (t == idx)
                                {
                                    return sectionSubInfo_[ii]._TypeParams[iii];
                                }
                                t++;
                            }
                        }
                        else
                        {
                            for (int iii = 0; iii < sectionSubInfo_[ii]._TypeReturn.size(); iii++)
                            {
                                if (t == idx)
                                {
                                    return sectionSubInfo_[ii]._TypeReturn[iii];
                                }
                                t++;
                            }
                        }
                    }
                }
            }
        }
    }

    if (fidx_ > 0)
    {
        for (int i = 0; i < sectionSubInfo_[fidx]._CodeLocalN.size(); i++)
        {
            for (int ii = 0; ii < sectionSubInfo_[fidx]._CodeLocalN[i]; ii++)
            {
                if (t == idx)
                {
                    return sectionSubInfo_[fidx]._CodeLocalType[i];
                }
                t++;
            }
        }
    }
    return 0;
}

int fileStructure::getVarTypeL(int idx, int fidx_, sectionSubInfo &sectionSubInfo__)
{
    int t = 0;
    int funcType = -1;
    for (int i = 0; i < sectionInfo_.size(); i++)
    {
        if (sectionInfo_[i].Id == 3)
        {
            for (int ii = sectionInfo_[i].SubIdx1; ii < sectionInfo_[i].SubIdx2; ii++)
            {
                if (sectionSubInfo_[ii].Index == sectionSubInfo__.Index)
                {
                    funcType = sectionSubInfo_[ii]._FunctionIdx;
                }
            }
        }
    }

    if (funcType >= 0)
    {
        for (int i = 0; i < sectionInfo_.size(); i++)
        {
            if (sectionInfo_[i].Id == 1)
            {
                for (int ii = sectionInfo_[i].SubIdx1; ii < sectionInfo_[i].SubIdx2; ii++)
                {
                    if (sectionSubInfo_[ii].Index == funcType)
                    {
                        if (fidx_ >= 0)
                        {
                            for (int iii = 0; iii < sectionSubInfo_[ii]._TypeParams.size(); iii++)
                            {
                                if (t == idx)
                                {
                                    return sectionSubInfo_[ii]._TypeParams[iii];
                                }
                                t++;
                            }
                        }
                        else
                        {
                            for (int iii = 0; iii < sectionSubInfo_[ii]._TypeReturn.size(); iii++)
                            {
                                if (t == idx)
                                {
                                    return sectionSubInfo_[ii]._TypeReturn[iii];
                                }
                                t++;
                            }
                        }
                    }
                }
            }
        }
    }

    if (fidx_ > 0)
    {
        for (int i = 0; i < sectionSubInfo__._CodeLocalN.size(); i++)
        {
            for (int ii = 0; ii < sectionSubInfo__._CodeLocalN[i]; ii++)
            {
                if (t == idx)
                {
                    return sectionSubInfo__._CodeLocalType[i];
                }
                t++;
            }
        }
    }
    return 0;
}

std::string fileStructure::getGlobalVarName(int num, bool def, int valType, bool rawname)
{
    /*for (int iii = 0; iii < sectionInfo_.size(); iii++)
    {
        // Name in import / Name in export
        if ((sectionInfo_[iii].Id == 2) || (sectionInfo_[iii].Id == 7))
        {
            for (int iiii = sectionInfo_[iii].SubIdx1; iiii < sectionInfo_[iii].SubIdx2; iiii++)
            {
                if (sectionSubInfo_[iiii]._FunctionTag == 3)
                {
                    if (sectionSubInfo_[iiii]._FunctionIdx == num)
                    {
                        if (rawname)
                        {
                            return sectionSubInfo_[iiii]._FunctionName;
                        }
                        else
                        {
                            return wasmDecompiler_.correctFunctionName(sectionSubInfo_[iiii]._FunctionName);
                        }
                    }
                }
            }
        }
    }*/

    if (rawname)
    {
        return wasmDecompiler_.metaTagGet(7, num, "");
    }

    if (def)
    {
        std::string varDispName = wasmDecompiler_.metaTagGet(107, num, "global" + std::to_string(num));
        if (wasmDecompiler_.decompOptVariableHungarian)
        {
            return wasmDecompiler_.valueTypeName(valType) + "_" + varDispName;
        }
        else
        {
            return varDispName;
        }
    }
    else
    {
        return "";
    }
}

int fileStructure::getTypeListItemByTag(int num)
{
    int tagNum = -1;
    for (int iii = 0; iii < sectionInfo_.size(); iii++)
    {
        if (sectionInfo_[iii].Id == 13)
        {
            for (int iiii = sectionInfo_[iii].SubIdx1; iiii < sectionInfo_[iii].SubIdx2; iiii++)
            {
                if (sectionSubInfo_[iiii].Index == num)
                {
                    tagNum = sectionSubInfo_[iiii]._TypeReturn[1];
                }
            }
        }
    }
    if (tagNum < 0)
    {
        return -1;
    }

    for (int iii = 0; iii < sectionInfo_.size(); iii++)
    {
        if (sectionInfo_[iii].Id == 1)
        {
            for (int iiii = sectionInfo_[iii].SubIdx1; iiii < sectionInfo_[iii].SubIdx2; iiii++)
            {
                if (sectionSubInfo_[iiii].Index == tagNum)
                {
                    return iiii;
                }
            }
        }
    }

    return -1;
}

std::string fileStructure::getFunctionNameById(getFunctionNameByIdMode mode, getFunctionNameByIdNumber number, int idx)
{
    int localNum_ = 0;
    return getFunctionNameByIdLocalNum(mode, number, idx, localNum_);
}

std::string fileStructure::getFunctionNameByIdLocalNum(getFunctionNameByIdMode mode, getFunctionNameByIdNumber number, int idx, int &localNum)
{
    std::string funcName = wasmDecompiler_.metaTagGet(101, idx, "function" + std::to_string(idx));
    int funcType = -1;

    // Calc import function count
    int importCount = 0;
    if ((number == getFunctionNameByIdNumber::whole) || (number == getFunctionNameByIdNumber::import))
    {
        for (int i_ = 0; i_ < sectionInfo_.size(); i_++)
        {
            if (sectionInfo_[i_].Id == 2) // Import - name and type
            {
                for (int ii0 = sectionInfo_[i_].SubIdx1; ii0 < sectionInfo_[i_].SubIdx2; ii0++)
                {
                    if (sectionSubInfo_[ii0]._FunctionTag == 0)
                    {
                        if (importCount < (sectionSubInfo_[ii0].Index + 1))
                        {
                            importCount = (sectionSubInfo_[ii0].Index + 1);
                        }
                        if (idx == sectionSubInfo_[ii0].Index)
                        {
                            //funcName = wasmDecompiler_.correctFunctionName(sectionSubInfo_[ii0]._FunctionName);
                            funcType = sectionSubInfo_[ii0]._CodeSize;
                        }
                    }
                }
            }
        }
        if (number == getFunctionNameByIdNumber::import)
        {
            return std::to_string(importCount);
        }
    }

    if (localNum > 0) { localNum = 0; }

    // find function name and type
    if (number == getFunctionNameByIdNumber::type)
    {
        funcName = "template";
        for (int i_ = 0; i_ < sectionInfo_.size(); i_++)
        {
            if (sectionInfo_[i_].Id == 1) // Type
            {
                for (int ii0 = sectionInfo_[i_].SubIdx1; ii0 < sectionInfo_[i_].SubIdx2; ii0++)
                {
                    if (idx == sectionSubInfo_[ii0].Index)
                    {
                        funcType = sectionSubInfo_[ii0].Index;
                    }
                }
            }
        }
    }
    else
    {
        for (int i_ = 0; i_ < sectionInfo_.size(); i_++)
        {
            /*if (sectionInfo_[i_].Id == 7) // Export - name only
            {
                for (int ii0 = sectionInfo_[i_].SubIdx1; ii0 < sectionInfo_[i_].SubIdx2; ii0++)
                {
                    if ((((idx - importCount) == sectionSubInfo_[ii0].Index)) && (sectionSubInfo_[ii0]._FunctionTag == 0))
                    {
                        funcName = wasmDecompiler_.correctFunctionName(sectionSubInfo_[ii0]._FunctionName);
                        customName = true;
                    }
                }
            }*/

            if (sectionInfo_[i_].Id == 3) // Function - type only
            {
                for (int ii0 = sectionInfo_[i_].SubIdx1; ii0 < sectionInfo_[i_].SubIdx2; ii0++)
                {
                    if ((idx - importCount) == sectionSubInfo_[ii0].Index)
                    {
                        funcType = sectionSubInfo_[ii0]._FunctionIdx;
                    }
                }
            }
        }
    }

    // Generate description, calc local variables
    std::string funcReturn = "void";
    std::string funcParam = "";
    if (funcType >= 0)
    {
        for (int i_ = 0; i_ < sectionInfo_.size(); i_++)
        {
            if (sectionInfo_[i_].Id == 1) // Type
            {
                for (int ii0 = sectionInfo_[i_].SubIdx1; ii0 < sectionInfo_[i_].SubIdx2; ii0++)
                {
                    if (funcType == sectionSubInfo_[ii0].Index)
                    {
                        if (sectionSubInfo_[ii0]._TypeReturn.size() > 0)
                        {
                            funcReturn = "";
                        }
                        for (int ii0i = 0; ii0i < sectionSubInfo_[ii0]._TypeReturn.size(); ii0i++)
                        {
                            if (ii0i > 0)
                            {
                                funcReturn = funcReturn + "_";
                            }
                            funcReturn = funcReturn + wasmDecompiler_.valueTypeName(sectionSubInfo_[ii0]._TypeReturn[ii0i]);
                        }
                        for (int ii0i = 0; ii0i < sectionSubInfo_[ii0]._TypeParams.size(); ii0i++)
                        {
                            if (ii0i > 0)
                            {
                                funcParam = funcParam + ", ";
                            }
                            funcParam = funcParam + wasmDecompiler_.valueTypeName(sectionSubInfo_[ii0]._TypeParams[ii0i]) + " ";
                            if (wasmDecompiler_.decompOptVariableHungarian)
                            {
                                funcParam = funcParam + wasmDecompiler_.valueTypeName(sectionSubInfo_[ii0]._TypeParams[ii0i]) + "_";
                            }
                            funcParam = funcParam + wasmDecompiler_.metaTagGet2(102, idx, localNum, "param" + std::to_string(localNum));
                            localNum++;
                        }
                    }
                }
            }
        }
        funcParam = "(" + funcParam + ")";
    }
    else
    {
        funcReturn = "error";
        funcParam = "(error)";
    }


    switch (mode)
    {
        case getFunctionNameByIdMode::debug:
            return funcReturn + " " + funcName + funcParam + " " + std::to_string(funcType) + " " + std::to_string(localNum);
        case getFunctionNameByIdMode::funcCode:
            return funcReturn + " " + funcName + funcParam;
        case getFunctionNameByIdMode::funcNameString:
            return funcName;
        case getFunctionNameByIdMode::funcTypeNumber:
            return std::to_string(funcType);
    }

    return "?";
}


std::string fileStructure::itemInfoText(int sectionId, sectionSubInfo &sectionSubInfo__)
{
    std::string info1 = wasmDecompiler_.htmlPrefix(sectionId + 100, sectionSubInfo__.Index) + "Item " + std::to_string(sectionSubInfo__.Index);
    std::string info2 = hex::IntToHex32(sectionSubInfo__.ItemAddr) + "-" + hex::IntToHex32(sectionSubInfo__.ItemAddr + sectionSubInfo__.ItemSize - 1) + wasmDecompiler_.htmlSuffix(sectionId + 100, sectionSubInfo__.Index);
    return info1 + ", " + info2 + ": ";
}

std::string fileStructure::sizeText(int minVal, int maxVal)
{
    if (maxVal < 0)
    {
        return std::to_string(minVal) + "..inf";
    }
    else
    {
        return std::to_string(minVal) + ".." + std::to_string(maxVal);
    }
}

void fileStructure::printCustom(stringBuf &sb, sectionInfo &sectionInfo__, bool infoItem, bool infoItemRaw, bool infoCode, int itemIndex)
{
    int nameLength = (sectionInfo__.StartIdx / 100);
    int nameNumSie = (sectionInfo__.StartIdx % 100);
    for (int ii = sectionInfo__.SubIdx1; ii < sectionInfo__.SubIdx2; ii++)
    {
        if ((itemIndex < 0) || (itemIndex == sectionSubInfo_[ii].Index))
        {
            if (itemIndex < 0) { sb.append(wasmDecompiler_.htmlPrefix(0, sectionSubInfo_[ii].Index)); }
            if (infoItem)
            {
                sb.append(itemInfoText(0, sectionSubInfo_[ii]));
                sb.append("custom[").append(sectionSubInfo_[ii].Index).append("] \"");
                int ptr = sectionInfo__.DataAddr + nameNumSie;
                std::string nameStr = "";
                for (int i = 0; i < nameLength; i++)
                {
                    if ((raw[ptr] >= 32) && (raw[ptr] <= 126))
                    {
                        nameStr.push_back(raw[ptr]);
                    }
                    else
                    {
                        nameStr.push_back('.');
                    }
                    ptr++;
                }
                sb.append(nameStr);
                sb.append("\" - ").append(sectionSubInfo_[ii]._CodeSize).append(" bytes");
                sb.eol();
            }
            if (infoItemRaw)
            {
                printRaw(sb, sectionSubInfo_[ii].ItemAddr, sectionSubInfo_[ii].ItemSize);
            }
            if (infoCode)
            {
                bool isName = false;
                if (wasmDecompiler_.useTags && (nameLength == 4) && (nameNumSie == 1))
                {
                    int ptr = sectionInfo__.DataAddr + nameNumSie;
                    if ((raw[ptr] == 'n') && (raw[ptr + 1] == 'a') && (raw[ptr + 2] == 'm') && (raw[ptr + 3] == 'e'))
                    {
                        isName = true;
                    }
                }
                if (isName && wasmDecompiler_.metaTagValid)
                {
                    int metaIdx = 0;
                    std::string metaItem = wasmDecompiler_.metaTagGetInfo(metaIdx);
                    while (metaItem != "`")
                    {
                        if (metaItem.size() > 1)
                        {
                            sb.append(metaItem).eol();
                        }
                        metaIdx++;
                        metaItem = wasmDecompiler_.metaTagGetInfo(metaIdx);
                    }
                }
                else
                {
                    printRaw(sb, sectionSubInfo_[ii]._CodeAddr, sectionSubInfo_[ii]._CodeSize);
                }
            }
            if (itemIndex < 0) { sb.append(wasmDecompiler_.htmlSuffix(0, sectionSubInfo_[ii].Index)); }
        }
    }

}

void fileStructure::printType(stringBuf &sb, sectionInfo &sectionInfo__, bool infoItem, bool infoItemRaw, bool infoCode, int itemIndex)
{
    for (int ii = sectionInfo__.SubIdx1; ii < sectionInfo__.SubIdx2; ii++)
    {
        if ((itemIndex < 0) || (itemIndex == sectionSubInfo_[ii].Index))
        {
            if (itemIndex < 0) { sb.append(wasmDecompiler_.htmlPrefix(1, sectionSubInfo_[ii].Index)); }
            if (infoItem)
            {
                sb.append(itemInfoText(1, sectionSubInfo_[ii]));
                sb.append("type[").append(sectionSubInfo_[ii].Index).append("]");
                sb.append(wasmDecompiler_.metaTagGetTempl(204, sectionSubInfo_[ii].Index, " ", ""));
                sb.append("  return = {");
                for (int iii = 0; iii < sectionSubInfo_[ii]._TypeReturn.size(); iii++)
                {
                    if (iii > 0) sb.append(", ");
                    sb.append(wasmDecompiler_.valueTypeName(sectionSubInfo_[ii]._TypeReturn[iii]));
                }
                sb.append("}  param = {");
                for (int iii = 0; iii < sectionSubInfo_[ii]._TypeParams.size(); iii++)
                {
                    if (iii > 0) sb.append(", ");
                    sb.append(wasmDecompiler_.valueTypeName(sectionSubInfo_[ii]._TypeParams[iii]));
                }
                sb.append("};");
                sb.eol();
            }
            if (infoCode)
            {
                printCodeText(sb, sectionSubInfo_[ii].ItemAddr, sectionSubInfo_[ii].ItemSize);
                sb.append(codeInstrInfoBlank());

                for (int iii = 0; iii < sectionSubInfo_[ii]._TypeReturn.size(); iii++)
                {
                    if (iii > 0)
                    {
                        sb.append("_");
                    }
                    sb.append(wasmDecompiler_.valueTypeName(sectionSubInfo_[ii]._TypeReturn[iii]));
                }
                if (sectionSubInfo_[ii]._TypeReturn.size() == 0) sb.append("void");
                sb.append(" template(");
                if (sectionSubInfo_[ii]._TypeParams.size() > 0)
                {
                    for (int iii = 0; iii < sectionSubInfo_[ii]._TypeParams.size(); iii++)
                    {
                        if (iii > 0)
                        {
                            sb.append(", ");
                        }
                        sb.append(wasmDecompiler_.valueTypeName(sectionSubInfo_[ii]._TypeParams[iii]));
                        sb.append(" ");
                        if (wasmDecompiler_.decompOptVariableHungarian)
                        {
                            sb.append(wasmDecompiler_.valueTypeName(sectionSubInfo_[ii]._TypeParams[iii])).append("_");
                        }
                        sb.append("param").append(iii);
                    }
                }
                else
                {
                    sb.append("void");
                }
                sb.append(");").eol();
            }
            if (infoItemRaw)
            {
                printRaw(sb, sectionSubInfo_[ii].ItemAddr, sectionSubInfo_[ii].ItemSize);
            }
            if (itemIndex < 0) { sb.append(wasmDecompiler_.htmlSuffix(1, sectionSubInfo_[ii].Index)); }
        }
    }
}

void fileStructure::printImport(stringBuf &sb, sectionInfo &sectionInfo__, bool infoItem, bool infoItemRaw, bool infoCode, int itemIndex)
{
    for (int ii = sectionInfo__.SubIdx1; ii < sectionInfo__.SubIdx2; ii++)
    {
        if ((itemIndex < 0) || (itemIndex == sectionSubInfo_[ii].Index))
        {
            if (itemIndex < 0) { sb.append(wasmDecompiler_.htmlPrefix(2, sectionSubInfo_[ii].Index)); }
            if (infoItem)
            {
                sb.append(itemInfoText(2, sectionSubInfo_[ii]));
                switch (sectionSubInfo_[ii]._FunctionTag)
                {
                    case 0: sb.append("function"); break;
                    case 1: sb.append("table"); break;
                    case 2: sb.append("memory"); break;
                    case 3: sb.append("global"); break;
                    default:sb.append("unknown"); break;
                }

                sb.append("[").append(sectionSubInfo_[ii]._FunctionIdx).append("] \"").append(sectionSubInfo_[ii]._FunctionName).append("\"");
                if (sectionSubInfo_[ii]._FunctionTag == 0)
                {
                    sb.append(" - type[").append(sectionSubInfo_[ii]._CodeSize).append("]");
                    sb.append(wasmDecompiler_.metaTagGetTempl(204, sectionSubInfo_[ii]._CodeSize, " ", ""));
                }
                if (sectionSubInfo_[ii]._FunctionTag == 1)
                {
                    sb.append(" ").append(sizeText(sectionSubInfo_[ii]._TypeReturn[1], sectionSubInfo_[ii]._TypeReturn[2]));
                    sb.append(" ").append(wasmDecompiler_.valueTypeName(sectionSubInfo_[ii]._TypeReturn[0]));
                }
                if (sectionSubInfo_[ii]._FunctionTag == 2)
                {
                    sb.append(" ").append(sizeText(sectionSubInfo_[ii]._TypeReturn[0], sectionSubInfo_[ii]._TypeReturn[1]));
                }
                sb.eol();
            }
            if (infoCode)
            {
                if (sectionSubInfo_[ii]._FunctionTag == 0)
                {
                    printCodeText(sb, sectionSubInfo_[ii].ItemAddr, sectionSubInfo_[ii].ItemSize);
                    sb.append(codeInstrInfoBlank());
                    sb.append(getFunctionNameById(getFunctionNameByIdMode::funcCode, getFunctionNameByIdNumber::whole, sectionSubInfo_[ii].Index)).append(";");
                    sb.eol();
                }
                if (sectionSubInfo_[ii]._FunctionTag == 3)
                {
                    printCodeText(sb, sectionSubInfo_[ii].ItemAddr, sectionSubInfo_[ii].ItemSize);
                    sb.append(codeInstrInfoBlank());
                    if (sectionSubInfo_[ii]._TypeReturn[1] == 0)
                    {
                        sb.append("const ");
                    }
                    sb.append(wasmDecompiler_.valueTypeName(sectionSubInfo_[ii]._TypeReturn[0])).append(" ").append(getGlobalVarName(sectionSubInfo_[ii]._FunctionIdx, true, sectionSubInfo_[ii]._TypeReturn[0], false)).append(";");
                    sb.eol();
                }
            }
            if (infoItemRaw)
            {
                printRaw(sb, sectionSubInfo_[ii].ItemAddr, sectionSubInfo_[ii].ItemSize);
            }
            if (itemIndex < 0) { sb.append(wasmDecompiler_.htmlSuffix(2, sectionSubInfo_[ii].Index)); }
        }
    }
}

void fileStructure::printFunction(stringBuf &sb, sectionInfo &sectionInfo__, bool infoItem, bool infoItemRaw, bool infoCode, int itemIndex)
{
    int importOffset = atoi(getFunctionNameById(getFunctionNameByIdMode::funcNameString, getFunctionNameByIdNumber::import, -1).c_str());
    for (int ii = sectionInfo__.SubIdx1; ii < sectionInfo__.SubIdx2; ii++)
    {
        if ((itemIndex < 0) || (itemIndex == sectionSubInfo_[ii].Index))
        {
            if (itemIndex < 0) { sb.append(wasmDecompiler_.htmlPrefix(3, sectionSubInfo_[ii].Index)); }
            if (infoItem)
            {
                sb.append(itemInfoText(3, sectionSubInfo_[ii]));
                sb.append("function[").append((sectionSubInfo_[ii].Index + importOffset)).append("]");
                sb.append(wasmDecompiler_.metaTagGetTempl(201, sectionSubInfo_[ii].Index + importOffset, " ", ""));
                sb.append(" - type[").append(sectionSubInfo_[ii]._FunctionIdx).append("]");
                sb.append(wasmDecompiler_.metaTagGetTempl(204, sectionSubInfo_[ii].Index + importOffset, " ", ""));
                sb.eol();
            }
            if (infoCode)
            {
                printCodeText(sb, sectionSubInfo_[ii].ItemAddr, sectionSubInfo_[ii].ItemSize);
                sb.append(codeInstrInfoBlank());
                sb.append(getFunctionNameById(getFunctionNameByIdMode::funcCode, getFunctionNameByIdNumber::whole, sectionSubInfo_[ii].Index + importOffset)).append(";");
                sb.eol();
            }
            if (infoItemRaw)
            {
                printRaw(sb, sectionSubInfo_[ii].ItemAddr, sectionSubInfo_[ii].ItemSize);
            }
            if (itemIndex < 0) { sb.append(wasmDecompiler_.htmlSuffix(3, sectionSubInfo_[ii].Index)); }
        }
    }
}

void fileStructure::printTable(stringBuf &sb, sectionInfo &sectionInfo__, bool infoItem, bool infoItemRaw, bool infoCode, int itemIndex)
{
    for (int ii = sectionInfo__.SubIdx1; ii < sectionInfo__.SubIdx2; ii++)
    {
        if ((itemIndex < 0) || (itemIndex == sectionSubInfo_[ii].Index))
        {
            if (itemIndex < 0) { sb.append(wasmDecompiler_.htmlPrefix(4, sectionSubInfo_[ii].Index)); }
            if (infoItem)
            {
                sb.append(itemInfoText(4, sectionSubInfo_[ii]));
                sb.append("table[").append(sectionSubInfo_[ii]._FunctionIdx).append("]");
                sb.append(wasmDecompiler_.metaTagGetTempl(205, sectionSubInfo_[ii].Index, " ", ""));
                sb.append(" ").append(sizeText(sectionSubInfo_[ii]._CodeLocalSize[0], sectionSubInfo_[ii]._CodeLocalSize[1]));
                sb.append(" ").append(wasmDecompiler_.valueTypeName(sectionSubInfo_[ii]._TypeReturn[0]));
                sb.eol();
            }
            if (infoCode)
            {
            }
            if (infoItemRaw)
            {
                printRaw(sb, sectionSubInfo_[ii].ItemAddr, sectionSubInfo_[ii].ItemSize);
            }
            if (itemIndex < 0) { sb.append(wasmDecompiler_.htmlSuffix(4, sectionSubInfo_[ii].Index)); }
        }
    }
}

void fileStructure::printMemory(stringBuf &sb, sectionInfo &sectionInfo__, bool infoItem, bool infoItemRaw, bool infoCode, int itemIndex)
{
    for (int ii = sectionInfo__.SubIdx1; ii < sectionInfo__.SubIdx2; ii++)
    {
        if ((itemIndex < 0) || (itemIndex == sectionSubInfo_[ii].Index))
        {
            if (itemIndex < 0) { sb.append(wasmDecompiler_.htmlPrefix(5, sectionSubInfo_[ii].Index)); }
            if (infoItem)
            {
                sb.append(itemInfoText(5, sectionSubInfo_[ii]));
                sb.append("memory[").append(sectionSubInfo_[ii].Index).append("]");
                sb.append(wasmDecompiler_.metaTagGetTempl(206, sectionSubInfo_[ii].Index, " ", ""));
                sb.append(" ").append(sizeText(sectionSubInfo_[ii]._CodeLocalSize[0], sectionSubInfo_[ii]._CodeLocalSize[1]));
                sb.eol();
            }
            if (infoCode)
            {
            }
            if (infoItemRaw)
            {
                printRaw(sb, sectionSubInfo_[ii].ItemAddr, sectionSubInfo_[ii].ItemSize);
            }
            if (itemIndex < 0) { sb.append(wasmDecompiler_.htmlSuffix(5, sectionSubInfo_[ii].Index)); }
        }
    }
}

void fileStructure::printGlobal(stringBuf &sb, sectionInfo &sectionInfo__, bool infoItem, bool infoItemRaw, bool infoCode, int decompType, int decompBranch, int itemIndex)
{
    for (int ii = sectionInfo__.SubIdx1; ii < sectionInfo__.SubIdx2; ii++)
    {
        if ((itemIndex < 0) || (itemIndex == sectionSubInfo_[ii].Index))
        {
            if (itemIndex < 0) { sb.append(wasmDecompiler_.htmlPrefix(6, sectionSubInfo_[ii].Index)); }
            if (infoItem)
            {
                sb.append(itemInfoText(6, sectionSubInfo_[ii]));
                sb.append("global[").append(sectionSubInfo_[ii]._FunctionIdx).append("]");
                sb.append(wasmDecompiler_.metaTagGetTempl(207, sectionSubInfo_[ii].Index, " ", ""));
                if (!sectionSubInfo_[ii]._CodeGood)
                {
                    sb.append(" !!! CODE PARSE ERROR !!!");
                }
                sb.eol();
            }
            if (infoCode)
            {
                printCodeText(sb, sectionSubInfo_[ii].Addr, 2);
                sb.append(codeInstrInfoBlank());
                std::string header = wasmDecompiler_.valueTypeName(sectionSubInfo_[ii]._TypeReturn[0]) + " " + getGlobalVarName(sectionSubInfo_[ii]._FunctionIdx, true, sectionSubInfo_[ii]._TypeReturn[0], false);
                if (sectionSubInfo_[ii]._TypeReturn[1] == 0)
                {
                    header = "const " + header;
                }

                if (decompType & 1)
                {
                    sb.append(wasmDecompiler_.htmlPrefix(306, sectionSubInfo_[ii].Index)).append(header).append(wasmDecompiler_.htmlSuffix(306, sectionSubInfo_[ii].Index)).eol();
                    printCodeText(sb, -1, 0);
                    sb.append(codeInstrInfoBlank());
                    sb.append("{");
                    sb.eol();
                }
                else
                {
                    sb.append(wasmDecompiler_.htmlPrefix(306, sectionSubInfo_[ii].Index)).append(header).append(";").append(wasmDecompiler_.htmlSuffix(306, sectionSubInfo_[ii].Index)).eol();
                }

                wasmDecompiler_.reset("*" + wasmDecompiler_.htmlPrefix(406, sectionSubInfo_[ii].Index) + header + ((decompType & 2) ? "" : ";") + wasmDecompiler_.htmlSuffix(406, sectionSubInfo_[ii].Index), decompBranch, sectionSubInfo_[ii]._CodeDataFieldDictionary);
                wasmDecompiler_.dataFieldDictionarySet("", sectionSubInfo_[ii]._TypeReturn[0], "return", 0);
                printCodeInstr(sb, sectionSubInfo_[ii]._CodeInstr, ii, decompType);
            }
            if (infoItemRaw)
            {
                printRaw(sb, sectionSubInfo_[ii].ItemAddr, sectionSubInfo_[ii].ItemSize);
            }
            if (itemIndex < 0) { sb.append(wasmDecompiler_.htmlSuffix(6, sectionSubInfo_[ii].Index)); }
        }
    }
}

void fileStructure::printExport(stringBuf &sb, sectionInfo &sectionInfo__, bool infoItem, bool infoItemRaw, bool infoCode, int itemIndex)
{
    for (int ii = sectionInfo__.SubIdx1; ii < sectionInfo__.SubIdx2; ii++)
    {
        if ((itemIndex < 0) || (itemIndex == sectionSubInfo_[ii].Index))
        {
            if (itemIndex < 0) { sb.append(wasmDecompiler_.htmlPrefix(7, sectionSubInfo_[ii].Index)); }
            if (infoItem)
            {
                sb.append(itemInfoText(7, sectionSubInfo_[ii]));
                switch (sectionSubInfo_[ii]._FunctionTag)
                {
                    case 0: sb.append("function"); break;
                    case 1: sb.append("table"); break;
                    case 2: sb.append("memory"); break;
                    case 3: sb.append("global"); break;
                    default:sb.append("unknown"); break;
                }

                sb.append("[").append(sectionSubInfo_[ii]._FunctionIdx).append("] \"").append(sectionSubInfo_[ii]._FunctionName).append("\"");
                sb.eol();
            }
            if (infoCode)
            {
                if (sectionSubInfo_[ii]._FunctionTag == 0)
                {
                    printCodeText(sb, sectionSubInfo_[ii].ItemAddr, sectionSubInfo_[ii].ItemSize);
                    sb.append(codeInstrInfoBlank());
                    sb.append(getFunctionNameById(getFunctionNameByIdMode::funcCode, getFunctionNameByIdNumber::internal, sectionSubInfo_[ii].Index)).append(";");
                    sb.eol();
                }
                if (sectionSubInfo_[ii]._FunctionTag == 3)
                {
                    printCodeText(sb, sectionSubInfo_[ii].ItemAddr, sectionSubInfo_[ii].ItemSize);
                    sb.append(codeInstrInfoBlank());
                    sb.append(wasmDecompiler_.valueTypeName(getVarTypeG(sectionSubInfo_[ii]._FunctionIdx))).append(" ").append(getGlobalVarName(sectionSubInfo_[ii]._FunctionIdx, true, getVarTypeG(sectionSubInfo_[ii]._FunctionIdx), false)).append(";");
                    sb.eol();
                }
            }
            if (infoItemRaw)
            {
                printRaw(sb, sectionSubInfo_[ii].ItemAddr, sectionSubInfo_[ii].ItemSize);
            }
            if (itemIndex < 0) { sb.append(wasmDecompiler_.htmlSuffix(7, sectionSubInfo_[ii].Index)); }
        }
    }
}

void fileStructure::printStart(stringBuf &sb, sectionInfo &sectionInfo__, bool infoItem, bool infoItemRaw, bool infoCode)
{
    sb.append("Entry point: function[").append(sectionInfo__.StartIdx).append("]");
    sb.append(" - ").append(getFunctionNameById(getFunctionNameByIdMode::funcCode, getFunctionNameByIdNumber::whole, sectionInfo__.StartIdx)).eol();
}

void fileStructure::printElement(stringBuf &sb, sectionInfo &sectionInfo__, bool infoItem, bool infoItemRaw, bool infoCode, int decompType, int decompBranch, int itemIndex)
{
    for (int ii = sectionInfo__.SubIdx1; ii < sectionInfo__.SubIdx2; ii++)
    {
        if ((itemIndex < 0) || (itemIndex == sectionSubInfo_[ii].Index))
        {
            if (itemIndex < 0) { sb.append(wasmDecompiler_.htmlPrefix(9, sectionSubInfo_[ii].Index)); }

            int itemCount = (sectionSubInfo_[ii]._TypeParams.size() - 2) / 2;

            if (infoItem)
            {
                sb.append(itemInfoText(9, sectionSubInfo_[ii]));
                sb.append("elem[").append(sectionSubInfo_[ii].Index).append("] ");
                sb.append(wasmDecompiler_.metaTagGetTempl(208, sectionSubInfo_[ii].Index, "", " "));
                switch (sectionSubInfo_[ii]._FunctionTag)
                {
                    case 0:
                        sb.append("active");
                        break;
                    case 1:
                        sb.append("active with table[").append(sectionSubInfo_[ii]._TypeParams[0]).append("]");
                        break;
                    case 2:
                        sb.append("passive");
                        break;
                    case 3:
                        sb.append("declare");
                        break;
                }
                switch (itemCount)
                {
                    case 0:
                        sb.append(", no items");
                        break;
                    case 1:
                        sb.append(", 1 item");
                        break;
                    case 2:
                        sb.append(", ").append(itemCount).append(" items");
                        break;
                }
                sb.eol();
            }
            if (infoCode)
            {
                std::string header = wasmDecompiler_.valueTypeName(wasmDecompiler_.fieldType_i32) + " elem" + std::to_string( sectionSubInfo_[ii].Index) + "_offset";
                if (sectionSubInfo_[ii]._CodeInstr.size() > 0)
                {
                    printCodeText(sb, sectionSubInfo_[ii].Addr, 1);
                    sb.append(codeInstrInfoBlank());

                    if (decompType & 1)
                    {
                        sb.append(wasmDecompiler_.htmlPrefix(309, sectionSubInfo_[ii].Index)).append(header).append(wasmDecompiler_.htmlSuffix(309, sectionSubInfo_[ii].Index)).eol();
                        printCodeText(sb, -1, 1);
                        sb.append(codeInstrInfoBlank());
                        sb.append("{");
                    }
                    else
                    {
                        sb.append(wasmDecompiler_.htmlPrefix(309, sectionSubInfo_[ii].Index)).append(header).append(";").append(wasmDecompiler_.htmlSuffix(309, sectionSubInfo_[ii].Index));
                    }
                    sb.eol();
                }

                wasmDecompiler_.reset("*" + wasmDecompiler_.htmlPrefix(409, sectionSubInfo_[ii].Index) + header + (((decompType & 2) ? "" : ";")) + wasmDecompiler_.htmlSuffix(409, sectionSubInfo_[ii].Index), decompBranch, sectionSubInfo_[ii]._CodeDataFieldDictionary);
                wasmDecompiler_.dataFieldDictionarySet("", 0, "return", 0);
                printCodeInstr(sb, sectionSubInfo_[ii]._CodeInstr, ii, decompType);

                for (int iii = 0; iii < itemCount; iii++)
                {
                    int printAddr = sectionSubInfo_[ii]._TypeParams[iii * 2 + 1];
                    int printIdx = sectionSubInfo_[ii]._TypeParams[iii * 2 + 2];
                    int printSize = sectionSubInfo_[ii]._TypeParams[iii * 2 + 3] - printAddr;
                    printCodeText(sb, printAddr, printSize);
                    sb.append(codeInstrInfoBlank());
                    sb.append(getFunctionNameById(getFunctionNameByIdMode::funcCode, getFunctionNameByIdNumber::whole, printIdx)).append(";");
                    sb.eol();
                }
            }
            if (infoItemRaw)
            {
                printRaw(sb, sectionSubInfo_[ii].ItemAddr, sectionSubInfo_[ii].ItemSize);
            }
            if (itemIndex < 0) { sb.append(wasmDecompiler_.htmlSuffix(9, sectionSubInfo_[ii].Index)); }
        }
    }
}

void fileStructure::printCode(stringBuf &sb, sectionInfo &sectionInfo__, bool infoItem, bool infoItemRaw, bool infoCode, int decompType, int decompBranch, int itemIndex)
{
    for (int ii = sectionInfo__.SubIdx1; ii < sectionInfo__.SubIdx2; ii++)
    {
        if ((itemIndex < 0) || (itemIndex == sectionSubInfo_[ii].Index))
        {
            if (itemIndex < 0) { sb.append(wasmDecompiler_.htmlPrefix(10, sectionSubInfo_[ii].Index)); }
            if (infoItem)
            {
                sb.append(itemInfoText(10, sectionSubInfo_[ii]));
                sb.append("function[").append(sectionSubInfo_[ii]._FunctionIdx).append("]");
                sb.append(wasmDecompiler_.metaTagGetTempl(201, sectionSubInfo_[ii]._FunctionIdx, " ", ""));
                if (!sectionSubInfo_[ii]._CodeGood)
                {
                    sb.append(" !!! CODE PARSE ERROR !!!");
                }
                sb.eol();
            }
            if (infoCode)
            {
                wasmDecompiler_.metaTagFunctionNumber = sectionSubInfo_[ii]._FunctionIdx;
                wasmDecompiler_.reset(((decompType & 2) ? "+" : "-") + wasmDecompiler_.htmlPrefix(410, sectionSubInfo_[ii].Index) + "|" + wasmDecompiler_.htmlSuffix(410, sectionSubInfo_[ii].Index) + "|" + getFunctionNameById(getFunctionNameByIdMode::funcNameString, getFunctionNameByIdNumber::whole, sectionSubInfo_[ii]._FunctionIdx) + "|", decompBranch, sectionSubInfo_[ii]._CodeDataFieldDictionary);
                {
                    int iii = 0;
                    while (getVarTypeG(iii) != 0)
                    {
                        wasmDecompiler_.dataFieldDictionarySet(getGlobalVarName(iii, false, 0, false), getVarTypeG(iii), "global", iii);
                        iii++;
                    }
                    iii = 0;
                    while (getVarTypeL(iii, 0 - ii) != 0)
                    {
                        wasmDecompiler_.dataFieldDictionarySet("", getVarTypeL(iii, 0 - ii), "return", iii);
                        iii++;
                    }
                }

                int localNum = 0;
                printCodeText(sb, sectionSubInfo_[ii].ItemAddr, sectionSubInfo_[ii]._CodeAddr_ - sectionSubInfo_[ii].ItemAddr);
                sb.append(codeInstrInfoBlank());
                sb.append(wasmDecompiler_.htmlPrefix(310, sectionSubInfo_[ii].Index));
                sb.append(getFunctionNameByIdLocalNum(getFunctionNameByIdMode::funcCode, getFunctionNameByIdNumber::whole, sectionSubInfo_[ii]._FunctionIdx, localNum));
                if ((decompType == 0) || (decompType == 2))
                {
                    sb.append(";");
                }
                sb.append(wasmDecompiler_.htmlSuffix(310, sectionSubInfo_[ii].Index));
                sb.eol();

                for (int iii = 0; iii < localNum; iii++)
                {
                    wasmDecompiler_.dataFieldDictionarySet("", getVarTypeL(iii, ii), "param", iii);
                }

                if (decompType & 1)
                {
                    printCodeText(sb, -1, 0);
                    sb.append(codeInstrInfoBlank());
                    sb.append("{");
                    sb.eol();
                }


                for (int iii = 0; iii < sectionSubInfo_[ii]._CodeLocalN.size(); iii++)
                {
                    if (decompType & 1)
                    {
                        printCodeText(sb, sectionSubInfo_[ii]._CodeLocalAddr[iii], sectionSubInfo_[ii]._CodeLocalSize[iii]);
                        sb.append(codeInstrInfoBlank());
                        sb.append(hex::indent(1)).append(wasmDecompiler_.valueTypeName(sectionSubInfo_[ii]._CodeLocalType[iii]));
                    }
                    for (int iiii = 0; iiii < sectionSubInfo_[ii]._CodeLocalN[iii]; iiii++)
                    {
                        std::string localVarId = wasmDecompiler_.dataFieldDictionarySet("", getVarTypeL(localNum, ii), "local", localNum);
                        if (decompType & 1)
                        {
                            if (iiii > 0)
                            {
                                sb.append(",");
                            }
                            sb.append(" ").append(wasmDecompiler_.dataFieldDictionaryDisplay(localVarId));
                        }
                        localNum++;
                    }
                    if (decompType & 1)
                    {
                        sb.append(";").eol();
                    }
                }
                printCodeInstr(sb, sectionSubInfo_[ii]._CodeInstr, ii, decompType);
            }
            if (infoItemRaw)
            {
                printRaw(sb, sectionSubInfo_[ii].ItemAddr, sectionSubInfo_[ii].ItemSize);
            }
            if (itemIndex < 0) { sb.append(wasmDecompiler_.htmlSuffix(10, sectionSubInfo_[ii].Index)); }
        }
    }
}

void fileStructure::printData(stringBuf &sb, sectionInfo &sectionInfo__, bool infoItem, bool infoItemRaw, bool infoCode, int decompType, int decompBranch, int itemIndex)
{
    for (int ii = sectionInfo__.SubIdx1; ii < sectionInfo__.SubIdx2; ii++)
    {
        if ((itemIndex < 0) || (itemIndex == sectionSubInfo_[ii].Index))
        {
            if (itemIndex < 0) { sb.append(wasmDecompiler_.htmlPrefix(11, sectionSubInfo_[ii].Index)); }
            if (infoItem)
            {
                sb.append(itemInfoText(11, sectionSubInfo_[ii]));
                sb.append("data[").append(sectionSubInfo_[ii].Index).append("]");
                sb.append(wasmDecompiler_.metaTagGetTempl(209, sectionSubInfo_[ii].Index, " ", ""));
                sb.append(" - ").append(sectionSubInfo_[ii]._CodeSize).append(" bytes");
                if (!sectionSubInfo_[ii]._CodeGood)
                {
                    sb.append(" !!! CODE PARSE ERROR !!!");
                }
                sb.eol();
            }
            if (infoCode)
            {
                std::string header = wasmDecompiler_.valueTypeName(wasmDecompiler_.fieldType_i32) + " data" + std::to_string( sectionSubInfo_[ii].Index) + "_offset";
                if (sectionSubInfo_[ii]._CodeInstr.size() > 0)
                {
                    printCodeText(sb, sectionSubInfo_[ii].Addr, 1);
                    sb.append(codeInstrInfoBlank());

                    if (decompType & 1)
                    {
                        sb.append(wasmDecompiler_.htmlPrefix(311, sectionSubInfo_[ii].Index)).append(header).append(wasmDecompiler_.htmlSuffix(311, sectionSubInfo_[ii].Index)).eol();
                        printCodeText(sb, -1, 1);
                        sb.append(codeInstrInfoBlank());
                        sb.append("{");
                    }
                    else
                    {
                        sb.append(wasmDecompiler_.htmlPrefix(311, sectionSubInfo_[ii].Index)).append(header).append(";").append(wasmDecompiler_.htmlSuffix(311, sectionSubInfo_[ii].Index));
                    }
                    sb.eol();
                }

                wasmDecompiler_.reset("*" + wasmDecompiler_.htmlPrefix(411, sectionSubInfo_[ii].Index) + header + (((decompType & 2) ? "" : ";")) + wasmDecompiler_.htmlSuffix(411, sectionSubInfo_[ii].Index), decompBranch, sectionSubInfo_[ii]._CodeDataFieldDictionary);
                wasmDecompiler_.dataFieldDictionarySet("", 0, "return", 0);
                printCodeInstr(sb, sectionSubInfo_[ii]._CodeInstr, ii, decompType);
                printRaw(sb, sectionSubInfo_[ii]._CodeAddr, sectionSubInfo_[ii]._CodeSize);
            }
            if (infoItemRaw)
            {
                printRaw(sb, sectionSubInfo_[ii].ItemAddr, sectionSubInfo_[ii].ItemSize);
            }
            if (itemIndex < 0) { sb.append(wasmDecompiler_.htmlSuffix(11, sectionSubInfo_[ii].Index)); }
        }
    }
}

void fileStructure::printDataCount(stringBuf &sb, sectionInfo &sectionInfo__, bool infoItem, bool infoItemRaw, bool infoCode)
{
    sb.append("Data count: ").append(sectionInfo__.StartIdx).eol();
}

void fileStructure::printTag(stringBuf &sb, sectionInfo &sectionInfo__, bool infoItem, bool infoItemRaw, bool infoCode, int itemIndex)
{
    for (int ii = sectionInfo__.SubIdx1; ii < sectionInfo__.SubIdx2; ii++)
    {
        if ((itemIndex < 0) || (itemIndex == sectionSubInfo_[ii].Index))
        {
            if (itemIndex < 0) { sb.append(wasmDecompiler_.htmlPrefix(13, sectionSubInfo_[ii].Index)); }
            if (infoItem)
            {
                sb.append(itemInfoText(13, sectionSubInfo_[ii]));
                sb.append("tag[").append(sectionSubInfo_[ii].Index).append("]");
                sb.append(wasmDecompiler_.metaTagGetTempl(211, sectionSubInfo_[ii].Index, " ", ""));
                sb.append("  ").append(sectionSubInfo_[ii]._TypeReturn[0]).append("  ").append(sectionSubInfo_[ii]._TypeReturn[1]);
                for (int i_ = 0; i_ < sectionInfo_.size(); i_++)
                {
                    if (sectionInfo_[i_].Id == 1)
                    {
                        for (int ii0 = sectionInfo_[i_].SubIdx1; ii0 < sectionInfo_[i_].SubIdx2; ii0++)
                        {
                            if (sectionSubInfo_[ii0].Index == sectionSubInfo_[ii]._TypeReturn[1])
                            {
                                sb.append("  return = {");
                                for (int iii = 0; iii < sectionSubInfo_[ii0]._TypeReturn.size(); iii++)
                                {
                                    if (iii > 0) sb.append(", ");
                                    sb.append(wasmDecompiler_.valueTypeName(sectionSubInfo_[ii0]._TypeReturn[iii]));
                                }
                                sb.append("}  param = {");
                                for (int iii = 0; iii < sectionSubInfo_[ii0]._TypeParams.size(); iii++)
                                {
                                    if (iii > 0) sb.append(", ");
                                    sb.append(wasmDecompiler_.valueTypeName(sectionSubInfo_[ii0]._TypeParams[iii]));
                                }
                                sb.append("};");
                            }
                        }
                    }
                }
                sb.eol();
            }
            if (infoItemRaw)
            {
                printRaw(sb, sectionSubInfo_[ii].ItemAddr, sectionSubInfo_[ii].ItemSize);
            }
            if (infoCode)
            {
                printCodeText(sb, sectionSubInfo_[ii].ItemAddr, sectionSubInfo_[ii].ItemSize);
                sb.append(codeInstrInfoBlank());


                for (int i_ = 0; i_ < sectionInfo_.size(); i_++)
                {
                    if (sectionInfo_[i_].Id == 1)
                    {
                        for (int ii0 = sectionInfo_[i_].SubIdx1; ii0 < sectionInfo_[i_].SubIdx2; ii0++)
                        {
                            if (sectionSubInfo_[ii0].Index == sectionSubInfo_[ii]._TypeReturn[1])
                            {
                                for (int iii = 0; iii < sectionSubInfo_[ii0]._TypeReturn.size(); iii++)
                                {
                                    if (iii > 0)
                                    {
                                        sb.append("_");
                                    }
                                    sb.append(wasmDecompiler_.valueTypeName(sectionSubInfo_[ii0]._TypeReturn[iii]));
                                }
                                if (sectionSubInfo_[ii0]._TypeReturn.size() == 0) sb.append("void");
                                sb.append(" template(");
                                if (sectionSubInfo_[ii0]._TypeParams.size() > 0)
                                {
                                    for (int iii = 0; iii < sectionSubInfo_[ii0]._TypeParams.size(); iii++)
                                    {
                                        if (iii > 0)
                                        {
                                            sb.append(", ");
                                        }
                                        sb.append(wasmDecompiler_.valueTypeName(sectionSubInfo_[ii0]._TypeParams[iii]));
                                        sb.append(" ");
                                        if (wasmDecompiler_.decompOptVariableHungarian)
                                        {
                                            sb.append(wasmDecompiler_.valueTypeName(sectionSubInfo_[ii0]._TypeParams[iii])).append("_");
                                        }
                                        sb.append("param").append(iii);
                                    }
                                }
                                else
                                {
                                    sb.append("void");
                                }
                                sb.append(");");
                            }
                        }
                    }
                }
                sb.eol();
            }
            if (itemIndex < 0) { sb.append(wasmDecompiler_.htmlSuffix(13, sectionSubInfo_[ii].Index)); }
        }
    }
}

void fileStructure::printRaw(stringBuf &sb, int rawAddr, int rawSize)
{
    int ptr = rawAddr;
    std::string blankByte = "--";
    std::string blankChar = "-";
    sb.append(hex::IntToHex32(ptr - (ptr & 15))).append("  ");
    std::string Str1 = "";
    std::string Str2 = " ";
    if ((rawAddr & 15) != 0)
    {
        for (int ii = (rawAddr & 15); ii > 0; ii--)
        {
            Str1 = Str1 + blankByte + " ";
            Str2 = Str2 + blankChar;
            if (ii == ((rawAddr & 15) - 7))
            {
                Str1.push_back(' ');
                Str2.push_back(' ');
            }
        }
    }
    for (int ii = 0; ii < rawSize; ii++)
    {
        Str1 = Str1 + hex::IntToHex8(raw[ptr]) + " ";
        if ((raw[ptr] >= 32) && (raw[ptr] <= 126))
        {
            Str2.push_back(raw[ptr]);
        }
        else
        {
            Str2.push_back('.');
        }

        ptr++;
        if (((ptr & 15) == 8) && (ii < (rawSize - 1)))
        {
            Str1.push_back(' ');
            Str2.push_back(' ');
        }

        if (((ptr & 15) == 0) && (ii < (rawSize - 1)))
        {
            sb.append(Str1).append(Str2).eol();
            sb.append(hex::IntToHex32(ptr)).append("  ");
            Str1 = "";
            Str2 = " ";
        }
    }
    if ((ptr & 15) > 0)
    {
        for (int ii = ((ptr & 15)); ii < 16; ii++)
        {
            Str1 = Str1 + blankByte + " ";
            Str2 = Str2 + blankChar;
            if (ii == 7)
            {
                Str1.push_back(' ');
                Str2.push_back(' ');
            }
        }
    }
    sb.append(Str1).append(Str2).eol();
}

std::string fileStructure::print(int codeBinSize_, int sectionId, bool infoRaw, bool infoItem, bool infoItemRaw, bool infoCode, int decompType, int decompBranch, int decompOpts, int itemIndex)
{
    switch (decompOpts & 3)
    {
        default:
            wasmDecompiler_.decompOptFold = false;
            wasmDecompiler_.decompOptStackSimplify = false;
            break;
        case 1:
            wasmDecompiler_.decompOptFold = false;
            wasmDecompiler_.decompOptStackSimplify = true;
            break;
        case 2:
            wasmDecompiler_.decompOptFold = true;
            wasmDecompiler_.decompOptStackSimplify = true;
            break;
        case 3:
            wasmDecompiler_.decompOptFold = true;
            wasmDecompiler_.decompOptStackSimplify = false;
            break;
    }

    wasmDecompiler_.decompOptVariableDeclare = decompOpts & 4;
    wasmDecompiler_.decompOptVariableHungarian = decompOpts & 8;
    wasmDecompiler_.codeInstrInfoStack = decompOpts & 16;

    codeBinSize = codeBinSize_;

    int totalItems = 0;

    stringBuf sb;
    sb.clear(wasmDecompiler_.useHtml);
    int ptr = 0;
    for (int i = 0; i < sectionInfo_.size(); i++)
    {
        bool toPrint = false;
        switch (sectionInfo_[i].Id)
        {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
            case 9:
            case 10:
            case 11:
            case 12:
            case 13:
                if (sectionInfo_[i].Id == sectionId)
                {
                    toPrint = true;
                }
                break;
            default: toPrint = (sectionId == 99); break;
        }
        if (toPrint)
        {
            ptr = sectionInfo_[i].Addr;

            if (itemIndex == (-1))
            {
                sb.append("Section type, address and size: ");
                switch (sectionInfo_[i].Id)
                {
                    case  0: sb.append("Custom (0)"); break;
                    case  1: sb.append("Type (1)"); break;
                    case  2: sb.append("Import (2)"); break;
                    case  3: sb.append("Function (3)"); break;
                    case  4: sb.append("Table (4)"); break;
                    case  5: sb.append("Memory (5)"); break;
                    case  6: sb.append("Global (6)"); break;
                    case  7: sb.append("Export (7)"); break;
                    case  8: sb.append("Start (8)"); break;
                    case  9: sb.append("Element (9)"); break;
                    case 10: sb.append("Code (10)"); break;
                    case 11: sb.append("Data (11)"); break;
                    case 12: sb.append("DataCount (12)"); break;
                    case 13: sb.append("Tag (13)"); break;
                    default: sb.append("Unknown (").append(sectionInfo_[i].Id).append(")"); break;
                    case 255:sb.append("Header"); break;
                }
                sb.append(", ").append(hex::IntToHex32(sectionInfo_[i].Addr));
                sb.append("-").append(hex::IntToHex32(sectionInfo_[i].Addr + sectionInfo_[i].Size - 1));
                sb.append(", ").append(sectionInfo_[i].Size).append(" bytes").eol();

                sb.append("Data address and size: ").append(hex::IntToHex32(sectionInfo_[i].DataAddr));
                sb.append("-").append(hex::IntToHex32(sectionInfo_[i].DataAddr + sectionInfo_[i].DataSize - 1));
                sb.append(", ").append(sectionInfo_[i].DataSize).append(" bytes").eol();
                switch (sectionInfo_[i].Id)
                {
                    case 1:
                    case 2:
                    case 3:
                    case 4:
                    case 5:
                    case 6:
                    case 7:
                    case 9:
                    case 10:
                    case 11:
                    case 13:
                        sb.append("Section items: ").append((sectionInfo_[i].SubIdx2 - sectionInfo_[i].SubIdx1)).eol();
                        break;
                    //case 8:
                    //case 12:
                    case 255:
                        printRaw(sb, sectionInfo_[i].Addr, sectionInfo_[i].Size);
                        break;
                }

                if (sectionInfo_[i].ParseStatus == 0)
                {
                    sb.append("!!! SECTION PARSE ERROR !!!").eol();
                }
                if (sectionInfo_[i].ParseStatus == 2)
                {
                    sb.append("Section parse is not implemented").eol();
                }

                if (infoRaw)
                {
                    printRaw(sb, sectionInfo_[i].Addr, sectionInfo_[i].Size);
                }
            }

            if ((itemIndex != (-2)) && (infoItem || infoCode || infoItemRaw))
            {
                switch (sectionInfo_[i].Id)
                {
                    case 0: printCustom(sb, sectionInfo_[i], infoItem, infoItemRaw, infoCode, itemIndex); break;
                    case 1: printType(sb, sectionInfo_[i], infoItem, infoItemRaw, infoCode, itemIndex); break;
                    case 2: printImport(sb, sectionInfo_[i], infoItem, infoItemRaw, infoCode, itemIndex); break;
                    case 3: printFunction(sb, sectionInfo_[i], infoItem, infoItemRaw, infoCode, itemIndex); break;
                    case 4: printTable(sb, sectionInfo_[i], infoItem, infoItemRaw, infoCode, itemIndex); break;
                    case 5: printMemory(sb, sectionInfo_[i], infoItem, infoItemRaw, infoCode, itemIndex); break;
                    case 6: printGlobal(sb, sectionInfo_[i], infoItem, infoItemRaw, infoCode, decompType, decompBranch, itemIndex); break;
                    case 7: printExport(sb, sectionInfo_[i], infoItem, infoItemRaw, infoCode, itemIndex); break;
                    case 8: printStart(sb, sectionInfo_[i], infoItem, infoItemRaw, infoCode); break;
                    case 9: printElement(sb, sectionInfo_[i], infoItem, infoItemRaw, infoCode, decompType, decompBranch, itemIndex); break;
                    case 10: printCode(sb, sectionInfo_[i], infoItem, infoItemRaw, infoCode, decompType, decompBranch, itemIndex); break;
                    case 11: printData(sb, sectionInfo_[i], infoItem, infoItemRaw, infoCode, decompType, decompBranch, itemIndex); break;
                    case 12: printDataCount(sb, sectionInfo_[i], infoItem, infoItemRaw, infoCode); break;
                    case 13: printTag(sb, sectionInfo_[i], infoItem, infoItemRaw, infoCode, itemIndex); break;
                }
            }

            switch (sectionInfo_[i].Id)
            {
                case 0:
                    totalItems++;
                    break;
                case 1:
                case 2:
                case 3:
                case 4:
                case 5:
                case 6:
                case 7:
                case 9:
                case 10:
                case 11:
                case 13:
                    totalItems = totalItems + (sectionInfo_[i].SubIdx2 - sectionInfo_[i].SubIdx1);
                    break;
            }
        }
        sb.flush();
    }
    if (itemIndex == (-2))
    {
        return std::to_string(totalItems);
    }
    std::string strX = sb.getString();
    return strX;
}
