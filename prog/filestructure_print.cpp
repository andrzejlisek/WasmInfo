#include "filestructure.h"

void fileStructure::printCodeText(std::stringstream &ss, int addr, int count)
{
    if (addr >= 0)
    {
        ss << hex::IntToHex32(addr) << ":";
        if (count > codeBinSize)
        {
            count = codeBinSize;
        }
        for (int i = 0; i < count; i++)
        {
            ss << " " << hex::IntToHex8(raw[addr + i]);
        }
        for (int i = count; i < codeBinSize; i++)
        {
            ss << "   ";
        }
    }
    else
    {
        ss << "         ";
        for (int i = 0; i < codeBinSize; i++)
        {
            ss << "   ";
        }
    }
    ss << "  ";
}

void fileStructure::printCodeInstr(std::stringstream &ss, std::vector<codeInstr> &codeInstr_, int fidx, int decompType)
{
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
                            typeIdx = atoi(getFunctionType(atoi(codeInstr_[iii].Param0.c_str()), "{~}", dummy).c_str());
                            break;
                        default:
                            typeIdx = atoi(getFunctionNameById(-1, atoi(codeInstr_[iii].Param0.c_str()), -3).c_str());
                            break;
                    }

                    int stackP = sectionSubInfo_[typeIdx]._TypeParams.size();
                    int stackR = sectionSubInfo_[typeIdx]._TypeReturn.size();
                    std::string stackInfo = std::to_string(stackP) + "`" + std::to_string(stackR) + "`" + getFunctionNameById(-1, atoi(codeInstr_[iii].Param0.c_str()), -1) + "`";
                    for (int iiii = 0; iiii < sectionSubInfo_[typeIdx]._TypeReturn.size(); iiii++)
                    {
                        stackInfo = stackInfo + std::to_string(sectionSubInfo_[typeIdx]._TypeReturn[iiii]) + "`";
                    }
                    wasmDecompiler_.addCommand(raw, codeInstr_[iii].Addr, codeInstr_[iii].Size, codeInstr_[iii].Param0, codeInstr_[iii].Param1, stackInfo);
                }
                break;
            // the functions requires artificial instructions to simulate propertly stack size within the block
            case 0x07: // catch
            case 0x08: // throw
                {
                    int catchTag = atoi(codeInstr_[iii].Param0.c_str());
                    int catchTypeIdx = getTypeListItemByTag(catchTag);
                    wasmDecompiler_.addCommand(raw, codeInstr_[iii].Addr, codeInstr_[iii].Size, codeInstr_[iii].Param0, codeInstr_[iii].Param1, codeInstr_[iii].Param2);

                    if (catchTypeIdx >= 0)
                    {
                        uchar * raw0 = new uchar[8];
                        if (sectionSubInfo_[catchTypeIdx]._TypeParams.size() > 0)
                        {
                            for (int iiii = 0; iiii < sectionSubInfo_[catchTypeIdx]._TypeParams.size(); iiii++)
                            {
                                switch (raw[codeInstr_[iii].Addr])
                                {
                                    case 0x07: // catch
                                        {
                                            switch (sectionSubInfo_[catchTypeIdx]._TypeParams[iiii])
                                            {
                                                default:   // wasmDecompiler_.fieldType_i32
                                                    raw0[0] = 0x41;
                                                    break;
                                                case 0x7E: // wasmDecompiler_.fieldType_i64
                                                    raw0[0] = 0x42;
                                                    break;
                                                case 0x7D: // wasmDecompiler_.fieldType_f32
                                                    raw0[0] = 0x43;
                                                    break;
                                                case 0x7C: // wasmDecompiler_.fieldType_f64
                                                    raw0[0] = 0x44;
                                                    break;
                                            }
                                            raw0[1] = 0;
                                            wasmDecompiler_.addCommand(raw0, 0, 2, "0", "", "");
                                        }
                                        break;
                                    case 0x08: // throw
                                        {
                                            raw0[0] = 0x1A;
                                            wasmDecompiler_.addCommand(raw0, 0, 1, "", "", "");
                                        }
                                        break;
                                }
                            }
                        }
                        delete[] raw0;
                    }

                }
                break;
            //case 0x02: // block
            //case 0x02: // loop
            //case 0x02: // if
            //    break;
            default:
                {
                    wasmDecompiler_.addCommand(raw, codeInstr_[iii].Addr, codeInstr_[iii].Size, codeInstr_[iii].Param0, codeInstr_[iii].Param1, codeInstr_[iii].Param2);
                }
                break;
        }

        if (decompType & 1)
        {
            printCodeText(ss, codeInstr_[iii].Addr, codeInstr_[iii].Size);
            std::string instrText = instructionText(codeInstr_[iii], fidx);
            int instrTextLine = instrText.find("\n");
            if (instrTextLine > 0)
            {
                while (instrTextLine > 0)
                {
                    ss << hex::indent(codeInstr_[iii].Depth) << instrText.substr(0, instrTextLine) << std::endl;
                    printCodeText(ss, -1, 0);
                    instrText = instrText.substr(instrTextLine + 1);
                    instrTextLine = instrText.find("\n");
                }
            }
            ss << hex::indent(codeInstr_[iii].Depth) << instrText << std::endl;
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
                printCodeText(ss, -1, 0);
                ss << decompInstr.substr(0, decompInstrLine) << std::endl;
                decompInstr = decompInstr.substr(decompInstrLine + 4);
                decompInstrLine = hex::StringIndexOf(decompInstr, "[\\n]");
            }

            printCodeText(ss, -1, 0);
            ss << decompInstr << std::endl;
            idx++;
            decompInstr = wasmDecompiler_.printCommand(idx);
        }
    }
}

std::string fileStructure::correctFunctionName(std::string funcName)
{
    std::string funcName0 = "";
    for (int i = 0; i < funcName.size(); i++)
    {
        bool std = false;
        if ((funcName[i] >= '0') && (funcName[i] <= '9')) std = true;
        if ((funcName[i] >= 'A') && (funcName[i] <= 'Z')) std = true;
        if ((funcName[i] >= 'a') && (funcName[i] <= 'z')) std = true;
        funcName0.push_back(std ? funcName[i] : '_');
    }
    return funcName0;
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

std::string fileStructure::getGlobalVarName(int num, bool def, int valType)
{
    for (int iii = 0; iii < sectionInfo_.size(); iii++)
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
                        return correctFunctionName(sectionSubInfo_[iiii]._FunctionName);
                    }
                }
            }
        }
    }

    if (def)
    {
        if (wasmDecompiler_.decompOptVariableHungarian)
        {
            return wasmDecompiler_.valueTypeName(valType) + "_global" + std::to_string(num);
        }
        else
        {
            return "global" + std::to_string(num);
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

std::string fileStructure::getFunctionType(int funcType, std::string funcName, int &localNum)
{
    switch (localNum)
    {
        case -1:
            return correctFunctionName(funcName);
        case -2:
            return std::to_string(funcType);
    }
    if (funcType >= 0)
    {
        std::string funcReturn = "void";
        std::string funcParam = "";
        for (int iii = 0; iii < sectionInfo_.size(); iii++)
        {
            if (sectionInfo_[iii].Id == 1)
            {
                for (int iiii = sectionInfo_[iii].SubIdx1; iiii < sectionInfo_[iii].SubIdx2; iiii++)
                {
                    if (sectionSubInfo_[iiii].Index == funcType)
                    {
                        if (localNum == -3)
                        {
                            return std::to_string(iiii);
                        }
                        if (sectionSubInfo_[iiii]._TypeReturn.size() > 0)
                        {
                            funcReturn = "";
                        }
                        for (int iiiii = 0; iiiii < sectionSubInfo_[iiii]._TypeReturn.size(); iiiii++)
                        {
                            if (iiiii > 0)
                            {
                                funcReturn = funcReturn + "_";
                            }
                            funcReturn = funcReturn + wasmDecompiler_.valueTypeName(sectionSubInfo_[iiii]._TypeReturn[iiiii]);
                        }
                        if (sectionSubInfo_[iiii]._TypeParams.size() == 0)
                        {
                            funcParam = "void";
                        }
                        for (int iiiii = 0; iiiii < sectionSubInfo_[iiii]._TypeParams.size(); iiiii++)
                        {
                            if (iiiii > 0)
                            {
                                funcParam = funcParam + ", ";
                            }
                            funcParam = funcParam + wasmDecompiler_.valueTypeName(sectionSubInfo_[iiii]._TypeParams[iiiii]) + " ";
                            if (wasmDecompiler_.decompOptVariableHungarian)
                            {
                                funcParam = funcParam + wasmDecompiler_.valueTypeName(sectionSubInfo_[iiii]._TypeParams[iiiii]) + "_";
                            }
                            funcParam = funcParam + "param" + std::to_string(localNum);
                            /*if (funcName == "{~}")
                            {
                                funcParam = funcParam + " param" + std::to_string(localNum);
                            }
                            else
                            {
                                funcParam = funcParam + " local" + std::to_string(localNum);
                            }*/
                            localNum++;
                        }
                    }
                }
            }
        }
        if (localNum == -3)
        {
            return "-1";
        }
        if (funcName == "{~}") funcName = "template";
        return funcReturn + " " + correctFunctionName(funcName) + "(" + funcParam + ")";
    }
    else
    {
        if (localNum == -3)
        {
            return "-1";
        }
        return "void " + correctFunctionName(funcName) + "()";
    }
}

std::string fileStructure::getFunctionName(int idx, int fidx, int &localNum)
{
    std::string funcName = "__function" + std::to_string(fidx);
    int funcType = -1;
    for (int iii = 0; iii < sectionInfo_.size(); iii++)
    {
        // Finding parameter and return types
        if (sectionInfo_[iii].Id == 3)
        {
            for (int iiii = sectionInfo_[iii].SubIdx1; iiii < sectionInfo_[iii].SubIdx2; iiii++)
            {
                if (sectionSubInfo_[iiii].Index == idx)
                {
                    funcType = sectionSubInfo_[iiii]._FunctionIdx;
                }
            }
        }

        // Finding function name
        if (sectionInfo_[iii].Id == 7)
        {
            for (int iiii = sectionInfo_[iii].SubIdx1; iiii < sectionInfo_[iii].SubIdx2; iiii++)
            {
                if (sectionSubInfo_[iiii]._FunctionTag == 0)
                {
                    if (sectionSubInfo_[iiii]._FunctionIdx == fidx)
                    {
                        funcName = sectionSubInfo_[iiii]._FunctionName;
                    }
                }
            }
        }
    }

    if (localNum > 0) { localNum = 0; }
    return getFunctionType(funcType, funcName, localNum);
}

std::string fileStructure::getFunctionNameById(int idx, int fidx, int localNum)
{
    // localNum == -1 Function name
    // localNum == -2 Function type index
    // localNum == -3 Function type element in sectionSubInfo_
    for (int i_ = 0; i_ < sectionInfo_.size(); i_++)
    {
        if (sectionInfo_[i_].Id == 2)
        {
            for (int ii0 = sectionInfo_[i_].SubIdx1; ii0 < sectionInfo_[i_].SubIdx2; ii0++)
            {
                if (((idx < 0) || (idx == sectionSubInfo_[ii0].Index)) && ((fidx < 0) || (fidx == sectionSubInfo_[ii0]._FunctionIdx)) && (sectionSubInfo_[ii0]._FunctionTag == 0))
                {
                    return getFunctionType(sectionSubInfo_[ii0]._CodeSize, sectionSubInfo_[ii0]._FunctionName, localNum);
                }
            }
        }
        if (sectionInfo_[i_].Id == 10)
        {
            for (int ii0 = sectionInfo_[i_].SubIdx1; ii0 < sectionInfo_[i_].SubIdx2; ii0++)
            {
                if (((idx < 0) || (idx == sectionSubInfo_[ii0].Index)) && ((fidx < 0) || (fidx == sectionSubInfo_[ii0]._FunctionIdx)))
                {
                    return getFunctionName(sectionSubInfo_[ii0].Index, sectionSubInfo_[ii0]._FunctionIdx, localNum);
                }
            }
        }
    }
    return "";
}


std::string fileStructure::itemInfoText(sectionSubInfo &sectionSubInfo__)
{
    return "Item " + std::to_string(sectionSubInfo__.Index) + ", " + hex::IntToHex32(sectionSubInfo__.ItemAddr) + "-" + hex::IntToHex32(sectionSubInfo__.ItemAddr + sectionSubInfo__.ItemSize - 1) + ": ";
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

void fileStructure::printType(std::stringstream &ss, sectionInfo &sectionInfo__, bool infoItem, bool infoItemRaw, bool infoCode)
{
    for (int ii = sectionInfo__.SubIdx1; ii < sectionInfo__.SubIdx2; ii++)
    {
        if (infoItem)
        {
            ss << itemInfoText(sectionSubInfo_[ii]);
            ss << "type[" << sectionSubInfo_[ii].Index << "]";
            ss << "  return = {";
            for (int iii = 0; iii < sectionSubInfo_[ii]._TypeReturn.size(); iii++)
            {
                if (iii > 0) ss << ", ";
                ss << wasmDecompiler_.valueTypeName(sectionSubInfo_[ii]._TypeReturn[iii]);
            }
            ss << "}  param = {";
            for (int iii = 0; iii < sectionSubInfo_[ii]._TypeParams.size(); iii++)
            {
                if (iii > 0) ss << ", ";
                ss << wasmDecompiler_.valueTypeName(sectionSubInfo_[ii]._TypeParams[iii]);
            }
            ss << "};";
            ss << std::endl;
        }
        if (infoItemRaw)
        {
            printRaw(ss, sectionSubInfo_[ii].ItemAddr, sectionSubInfo_[ii].ItemSize);
        }
        if (infoCode)
        {
            printCodeText(ss, sectionSubInfo_[ii].ItemAddr, sectionSubInfo_[ii].ItemSize);

            for (int iii = 0; iii < sectionSubInfo_[ii]._TypeReturn.size(); iii++)
            {
                if (iii > 0)
                {
                    ss << "_";
                }
                ss << wasmDecompiler_.valueTypeName(sectionSubInfo_[ii]._TypeReturn[iii]);
            }
            if (sectionSubInfo_[ii]._TypeReturn.size() == 0) ss << "void";
            ss << " template(";
            if (sectionSubInfo_[ii]._TypeParams.size() > 0)
            {
                for (int iii = 0; iii < sectionSubInfo_[ii]._TypeParams.size(); iii++)
                {
                    if (iii > 0)
                    {
                        ss << ", ";
                    }
                    ss << wasmDecompiler_.valueTypeName(sectionSubInfo_[ii]._TypeParams[iii]);
                    ss << " ";
                    if (wasmDecompiler_.decompOptVariableHungarian)
                    {
                        ss << wasmDecompiler_.valueTypeName(sectionSubInfo_[ii]._TypeParams[iii]) << "_";
                    }
                    ss << "param" << iii;
                }
            }
            else
            {
                ss << "void";
            }
            ss << ");" << std::endl;
        }
    }
}

void fileStructure::printImport(std::stringstream &ss, sectionInfo &sectionInfo__, bool infoItem, bool infoItemRaw, bool infoCode)
{
    for (int ii = sectionInfo__.SubIdx1; ii < sectionInfo__.SubIdx2; ii++)
    {
        if (infoItem)
        {
            ss << itemInfoText(sectionSubInfo_[ii]);
            switch (sectionSubInfo_[ii]._FunctionTag)
            {
                case 0: ss << "function"; break;
                case 1: ss << "table"; break;
                case 2: ss << "memory"; break;
                case 3: ss << "global"; break;
                default:ss << "unknown"; break;
            }

            ss << "[" << sectionSubInfo_[ii]._FunctionIdx << "] \"" << sectionSubInfo_[ii]._FunctionName + "\"";
            if (sectionSubInfo_[ii]._FunctionTag == 1)
            {
                ss << " " << sizeText(sectionSubInfo_[ii]._TypeReturn[1], sectionSubInfo_[ii]._TypeReturn[2]);
                ss << " " << wasmDecompiler_.valueTypeName(sectionSubInfo_[ii]._TypeReturn[0]);
            }
            if (sectionSubInfo_[ii]._FunctionTag == 2)
            {
                ss << " " << sizeText(sectionSubInfo_[ii]._TypeReturn[0], sectionSubInfo_[ii]._TypeReturn[1]);
            }
            ss << std::endl;
        }
        if (infoItemRaw)
        {
            printRaw(ss, sectionSubInfo_[ii].ItemAddr, sectionSubInfo_[ii].ItemSize);
        }
        if (infoCode)
        {
            if (sectionSubInfo_[ii]._FunctionTag == 0)
            {
                printCodeText(ss, sectionSubInfo_[ii].ItemAddr, sectionSubInfo_[ii].ItemSize);
                ss << getFunctionNameById(-1, sectionSubInfo_[ii]._FunctionIdx, 0) << ";";
                ss << std::endl;
            }
            if (sectionSubInfo_[ii]._FunctionTag == 3)
            {
                printCodeText(ss, sectionSubInfo_[ii].ItemAddr, sectionSubInfo_[ii].ItemSize);
                if (sectionSubInfo_[ii]._TypeReturn[1] == 0)
                {
                    ss << "const ";
                }
                ss << wasmDecompiler_.valueTypeName(sectionSubInfo_[ii]._TypeReturn[0]) << " " << getGlobalVarName(sectionSubInfo_[ii]._FunctionIdx, true, sectionSubInfo_[ii]._TypeReturn[0]) << ";";
                ss << std::endl;
            }
        }
    }
}

void fileStructure::printFunction(std::stringstream &ss, sectionInfo &sectionInfo__, bool infoItem, bool infoItemRaw, bool infoCode)
{
    for (int ii = sectionInfo__.SubIdx1; ii < sectionInfo__.SubIdx2; ii++)
    {
        if (infoItem)
        {
            ss << itemInfoText(sectionSubInfo_[ii]) << sectionSubInfo_[ii]._FunctionIdx;
            ss << std::endl;
        }
        if (infoItemRaw)
        {
            printRaw(ss, sectionSubInfo_[ii].ItemAddr, sectionSubInfo_[ii].ItemSize);
        }
        if (infoCode)
        {
            printCodeText(ss, sectionSubInfo_[ii].ItemAddr, sectionSubInfo_[ii].ItemSize);
            ss << getFunctionNameById(sectionSubInfo_[ii].Index, -1, 0) << ";";
            ss << std::endl;
        }
    }
}

void fileStructure::printTable(std::stringstream &ss, sectionInfo &sectionInfo__, bool infoItem, bool infoItemRaw, bool infoCode)
{
    for (int ii = sectionInfo__.SubIdx1; ii < sectionInfo__.SubIdx2; ii++)
    {
        if (infoItem)
        {
            ss << itemInfoText(sectionSubInfo_[ii]);
            ss << "table[" << sectionSubInfo_[ii]._FunctionIdx << "]";
            ss << " " << sizeText(sectionSubInfo_[ii]._CodeLocalSize[0], sectionSubInfo_[ii]._CodeLocalSize[1]);
            ss << " " << wasmDecompiler_.valueTypeName(sectionSubInfo_[ii]._TypeReturn[0]);
            ss << std::endl;
        }
        if (infoItemRaw)
        {
            printRaw(ss, sectionSubInfo_[ii].ItemAddr, sectionSubInfo_[ii].ItemSize);
        }
    }
}

void fileStructure::printMemory(std::stringstream &ss, sectionInfo &sectionInfo__, bool infoItem, bool infoItemRaw, bool infoCode)
{
    for (int ii = sectionInfo__.SubIdx1; ii < sectionInfo__.SubIdx2; ii++)
    {
        if (infoItem)
        {
            ss << itemInfoText(sectionSubInfo_[ii]);
            ss << "memory[" << sectionSubInfo_[ii].Index << "]";
            ss << " " << sizeText(sectionSubInfo_[ii]._CodeLocalSize[0], sectionSubInfo_[ii]._CodeLocalSize[1]);
            ss << std::endl;
        }
        if (infoItemRaw)
        {
            printRaw(ss, sectionSubInfo_[ii].ItemAddr, sectionSubInfo_[ii].ItemSize);
        }
    }
}

void fileStructure::printGlobal(std::stringstream &ss, sectionInfo &sectionInfo__, bool infoItem, bool infoItemRaw, bool infoCode, int decompType, int decompBranch)
{
    for (int ii = sectionInfo__.SubIdx1; ii < sectionInfo__.SubIdx2; ii++)
    {
        if (infoItem)
        {
            ss << itemInfoText(sectionSubInfo_[ii]);
            ss << "global[" << sectionSubInfo_[ii]._FunctionIdx << "]";
            if (!sectionSubInfo_[ii]._CodeGood)
            {
                ss << " !!! CODE PARSE ERROR !!!";
            }
            ss << std::endl;
        }
        if (infoItemRaw)
        {
            printRaw(ss, sectionSubInfo_[ii].ItemAddr, sectionSubInfo_[ii].ItemSize);
        }
        if (infoCode)
        {
            printCodeText(ss, sectionSubInfo_[ii].Addr, 2);
            if (sectionSubInfo_[ii]._TypeReturn[1] == 0)
            {
                ss << "const ";
            }
            ss << wasmDecompiler_.valueTypeName(sectionSubInfo_[ii]._TypeReturn[0]) << " " << getGlobalVarName(sectionSubInfo_[ii]._FunctionIdx, true, sectionSubInfo_[ii]._TypeReturn[0]) << ";";
            ss << std::endl;

            if (decompType & 1)
            {
                printCodeText(ss, -1, 0);
                ss << "{";
                ss << std::endl;
            }

            wasmDecompiler_.reset("", decompBranch, sectionSubInfo_[ii]._CodeDataFieldDictionary);
            wasmDecompiler_.dataFieldDictionarySet("", sectionSubInfo_[ii]._TypeReturn[0], "return", 0);
            printCodeInstr(ss, sectionSubInfo_[ii]._CodeInstr, ii, decompType);
        }
    }
}

void fileStructure::printExport(std::stringstream &ss, sectionInfo &sectionInfo__, bool infoItem, bool infoItemRaw, bool infoCode)
{
    for (int ii = sectionInfo__.SubIdx1; ii < sectionInfo__.SubIdx2; ii++)
    {
        if (infoItem)
        {
            ss << itemInfoText(sectionSubInfo_[ii]);
            switch (sectionSubInfo_[ii]._FunctionTag)
            {
                case 0: ss << "function"; break;
                case 1: ss << "table"; break;
                case 2: ss << "memory"; break;
                case 3: ss << "global"; break;
                default:ss << "unknown"; break;
            }

            ss << "[" << sectionSubInfo_[ii]._FunctionIdx << "] \"" << sectionSubInfo_[ii]._FunctionName << "\"";
            ss << std::endl;
        }
        if (infoItemRaw)
        {
            printRaw(ss, sectionSubInfo_[ii].ItemAddr, sectionSubInfo_[ii].ItemSize);
        }
        if (infoCode)
        {
            if (sectionSubInfo_[ii]._FunctionTag == 0)
            {
                printCodeText(ss, sectionSubInfo_[ii].ItemAddr, sectionSubInfo_[ii].ItemSize);
                ss << getFunctionNameById(-1, sectionSubInfo_[ii]._FunctionIdx, 0) << ";";
                ss << std::endl;
            }
            if (sectionSubInfo_[ii]._FunctionTag == 3)
            {
                printCodeText(ss, sectionSubInfo_[ii].ItemAddr, sectionSubInfo_[ii].ItemSize);
                ss << wasmDecompiler_.valueTypeName(getVarTypeG(sectionSubInfo_[ii]._FunctionIdx)) << " " << getGlobalVarName(sectionSubInfo_[ii]._FunctionIdx, true, getVarTypeG(sectionSubInfo_[ii]._FunctionIdx)) << ";";
                ss << std::endl;
            }
        }
    }
}

void fileStructure::printStart(std::stringstream &ss, sectionInfo &sectionInfo__, bool infoItem, bool infoItemRaw, bool infoCode)
{
    ss << "Entry point: function[" << sectionInfo__.StartIdx << "]";
    ss << " - " << getFunctionNameById(-1, sectionInfo__.StartIdx, 0) << std::endl;
}

void fileStructure::printElement(std::stringstream &ss, sectionInfo &sectionInfo__, bool infoItem, bool infoItemRaw, bool infoCode)
{

}

void fileStructure::printCode(std::stringstream &ss, sectionInfo &sectionInfo__, bool infoItem, bool infoItemRaw, bool infoCode, int decompType, int decompBranch)
{
    for (int ii = sectionInfo__.SubIdx1; ii < sectionInfo__.SubIdx2; ii++)
    {
        if (infoItem)
        {
            ss << itemInfoText(sectionSubInfo_[ii]);
            ss << "function[" << sectionSubInfo_[ii]._FunctionIdx << "]";
            if (!sectionSubInfo_[ii]._CodeGood)
            {
                ss << " !!! CODE PARSE ERROR !!!";
            }
            //ss << " Offset:" << hex::IntToHex32(sectionSubInfo_[ii]._CodeAddr);
            //ss << " Size:" << hex::IntToHex32(sectionSubInfo_[ii]._CodeSize) << "=" << sectionSubInfo_[ii]._CodeSize;
            ss << std::endl;
        }
        if (infoItemRaw)
        {
            printRaw(ss, sectionSubInfo_[ii].ItemAddr, sectionSubInfo_[ii].ItemSize);
        }
        if (infoCode)
        {
            int localNum = -1;
            wasmDecompiler_.reset(getFunctionName(sectionSubInfo_[ii].Index, sectionSubInfo_[ii]._FunctionIdx, localNum), decompBranch, sectionSubInfo_[ii]._CodeDataFieldDictionary);
            if ((decompType & 1) || (decompType == 0))
            {
                printCodeText(ss, sectionSubInfo_[ii].ItemAddr, sectionSubInfo_[ii]._CodeAddr_ - sectionSubInfo_[ii].ItemAddr);
            }

            {
                int iii = 0;
                while (getVarTypeG(iii) != 0)
                {
                    wasmDecompiler_.dataFieldDictionarySet(getGlobalVarName(iii, false, 0), getVarTypeG(iii), "global", iii);
                    iii++;
                }
                iii = 0;
                while (getVarTypeL(iii, 0 - ii) != 0)
                {
                    wasmDecompiler_.dataFieldDictionarySet("", getVarTypeL(iii, 0 - ii), "return", iii);
                    iii++;
                }
            }

            localNum = 0;
            if ((decompType & 1) || (decompType == 0))
            {
                ss << getFunctionName(sectionSubInfo_[ii].Index, sectionSubInfo_[ii]._FunctionIdx, localNum);
                if (decompType == 0)
                {
                    ss << ";";
                }
                ss << std::endl;
            }
            else
            {
                // Necessary for get correct localNum value
                getFunctionName(sectionSubInfo_[ii].Index, sectionSubInfo_[ii]._FunctionIdx, localNum);
            }

            for (int iii = 0; iii < localNum; iii++)
            {
                wasmDecompiler_.dataFieldDictionarySet("", getVarTypeL(iii, ii), "param", iii);
            }

            if (decompType & 1)
            {
                printCodeText(ss, -1, 0);
                ss << "{";
                ss << std::endl;
            }


            for (int iii = 0; iii < sectionSubInfo_[ii]._CodeLocalN.size(); iii++)
            {
                if (decompType & 1)
                {
                    printCodeText(ss, sectionSubInfo_[ii]._CodeLocalAddr[iii], sectionSubInfo_[ii]._CodeLocalSize[iii]);
                    ss << hex::indent(1) << wasmDecompiler_.valueTypeName(sectionSubInfo_[ii]._CodeLocalType[iii]);
                }
                for (int iiii = 0; iiii < sectionSubInfo_[ii]._CodeLocalN[iii]; iiii++)
                {
                    std::string localVarId = wasmDecompiler_.dataFieldDictionarySet("", getVarTypeL(localNum, ii), "local", localNum);
                    if (decompType & 1)
                    {
                        if (iiii > 0)
                        {
                            ss << ",";
                        }
                        ss << " " << wasmDecompiler_.dataFieldDictionaryDisplay(localVarId);
                    }
                    localNum++;
                }
                if (decompType & 1)
                {
                    ss << ";" << std::endl;
                }
            }
            printCodeInstr(ss, sectionSubInfo_[ii]._CodeInstr, ii, decompType);
        }
    }
}

void fileStructure::printData(std::stringstream &ss, sectionInfo &sectionInfo__, bool infoItem, bool infoItemRaw, bool infoCode, int decompType, int decompBranch)
{
    for (int ii = sectionInfo__.SubIdx1; ii < sectionInfo__.SubIdx2; ii++)
    {
        if (infoItem)
        {
            ss << itemInfoText(sectionSubInfo_[ii]);
            ss << "data[" << sectionSubInfo_[ii].Index << "] - " << sectionSubInfo_[ii]._CodeSize << " bytes";
            if (!sectionSubInfo_[ii]._CodeGood)
            {
                ss << " !!! CODE PARSE ERROR !!!";
            }
            ss << std::endl;
        }
        if (infoItemRaw)
        {
            printRaw(ss, sectionSubInfo_[ii].ItemAddr, sectionSubInfo_[ii].ItemSize);
        }
        if (infoCode)
        {
            if (decompType & 1)
            {
                printCodeText(ss, sectionSubInfo_[ii].Addr, 1);
                if (sectionSubInfo_[ii]._CodeInstr.size() > 0)
                {
                    ss << "{";
                }
                ss << std::endl;
            }

            wasmDecompiler_.reset("", decompBranch, sectionSubInfo_[ii]._CodeDataFieldDictionary);
            wasmDecompiler_.dataFieldDictionarySet("", 0, "return", 0);
            printCodeInstr(ss, sectionSubInfo_[ii]._CodeInstr, ii, decompType);
            printRaw(ss, sectionSubInfo_[ii]._CodeAddr, sectionSubInfo_[ii]._CodeSize);
        }
    }
}

void fileStructure::printDataCount(std::stringstream &ss, sectionInfo &sectionInfo__, bool infoItem, bool infoItemRaw, bool infoCode)
{
    ss << "Data count: " << sectionInfo__.StartIdx << std::endl;
}

void fileStructure::printTag(std::stringstream &ss, sectionInfo &sectionInfo__, bool infoItem, bool infoItemRaw, bool infoCode)
{
    for (int ii = sectionInfo__.SubIdx1; ii < sectionInfo__.SubIdx2; ii++)
    {
        if (infoItem)
        {
            ss << itemInfoText(sectionSubInfo_[ii]);
            ss << "tag[" << sectionSubInfo_[ii].Index << "]  ";
            ss << sectionSubInfo_[ii]._TypeReturn[0] << "  " << sectionSubInfo_[ii]._TypeReturn[1];
            for (int i_ = 0; i_ < sectionInfo_.size(); i_++)
            {
                if (sectionInfo_[i_].Id == 1)
                {
                    for (int ii0 = sectionInfo_[i_].SubIdx1; ii0 < sectionInfo_[i_].SubIdx2; ii0++)
                    {
                        if (sectionSubInfo_[ii0].Index == sectionSubInfo_[ii]._TypeReturn[1])
                        {
                            ss << "  return = {";
                            for (int iii = 0; iii < sectionSubInfo_[ii0]._TypeReturn.size(); iii++)
                            {
                                if (iii > 0) ss << ", ";
                                ss << wasmDecompiler_.valueTypeName(sectionSubInfo_[ii0]._TypeReturn[iii]);
                            }
                            ss << "}  param = {";
                            for (int iii = 0; iii < sectionSubInfo_[ii0]._TypeParams.size(); iii++)
                            {
                                if (iii > 0) ss << ", ";
                                ss << wasmDecompiler_.valueTypeName(sectionSubInfo_[ii0]._TypeParams[iii]);
                            }
                            ss << "};";

                        }
                    }
                }
            }
            ss << std::endl;
        }
        if (infoItemRaw)
        {
            printRaw(ss, sectionSubInfo_[ii].ItemAddr, sectionSubInfo_[ii].ItemSize);
        }
        if (infoCode)
        {
            printCodeText(ss, sectionSubInfo_[ii].ItemAddr, sectionSubInfo_[ii].ItemSize);


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
                                    ss << "_";
                                }
                                ss << wasmDecompiler_.valueTypeName(sectionSubInfo_[ii0]._TypeReturn[iii]);
                            }
                            if (sectionSubInfo_[ii0]._TypeReturn.size() == 0) ss << "void";
                            ss << " template(";
                            if (sectionSubInfo_[ii0]._TypeParams.size() > 0)
                            {
                                for (int iii = 0; iii < sectionSubInfo_[ii0]._TypeParams.size(); iii++)
                                {
                                    if (iii > 0)
                                    {
                                        ss << ", ";
                                    }
                                    ss << wasmDecompiler_.valueTypeName(sectionSubInfo_[ii0]._TypeParams[iii]);
                                    ss << " ";
                                    if (wasmDecompiler_.decompOptVariableHungarian)
                                    {
                                        ss << wasmDecompiler_.valueTypeName(sectionSubInfo_[ii0]._TypeParams[iii]) << "_";
                                    }
                                    ss << "param" << iii;
                                }
                            }
                            else
                            {
                                ss << "void";
                            }
                            ss << ");";
                        }
                    }
                }
            }
            ss << std::endl;
        }
    }
}

void fileStructure::printRaw(std::stringstream &ss, int rawAddr, int rawSize)
{
    int ptr = rawAddr;
    std::string blankByte = "--";
    std::string blankChar = "-";
    ss << hex::IntToHex32(ptr - (ptr & 15)) << "  ";
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
            ss << Str1 << Str2 << std::endl;
            ss << hex::IntToHex32(ptr) << "  ";
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
    ss << Str1 << Str2 << std::endl;
}

std::string fileStructure::print(int codeBinSize_, int sectionId, bool infoRaw, bool infoItem, bool infoItemRaw, bool infoCode, int decompType, int decompBranch, int decompOpts)
{
    wasmDecompiler_.decompOptFold = decompOpts & 1;
    wasmDecompiler_.decompOptStackSimplify = decompOpts & 2;
    wasmDecompiler_.decompOptVariableDeclare = decompOpts & 4;
    wasmDecompiler_.decompOptVariableHungarian = decompOpts & 8;

    codeBinSize = codeBinSize_;

    std::stringstream ss;
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

            if (true)
            {
                ss << "Section type, address and size: ";
                switch (sectionInfo_[i].Id)
                {
                    case  0: ss << "Custom (0)"; break;
                    case  1: ss << "Type (1)"; break;
                    case  2: ss << "Import (2)"; break;
                    case  3: ss << "Function (3)"; break;
                    case  4: ss << "Table (4)"; break;
                    case  5: ss << "Memory (5)"; break;
                    case  6: ss << "Global (6)"; break;
                    case  7: ss << "Export (7)"; break;
                    case  8: ss << "Start (8)"; break;
                    case  9: ss << "Element (9)"; break;
                    case 10: ss << "Code (10)"; break;
                    case 11: ss << "Data (11)"; break;
                    case 12: ss << "DataCount (12)"; break;
                    case 13: ss << "Tag (13)"; break;
                    default: ss << "Unknown (" << sectionInfo_[i].Id << ")"; break;
                    case 255:ss << "Header"; break;
                }
                ss << ", " << hex::IntToHex32(sectionInfo_[i].Addr);
                ss << "-" << hex::IntToHex32(sectionInfo_[i].Addr + sectionInfo_[i].Size - 1);
                ss << ", " << sectionInfo_[i].Size << " bytes" << std::endl;

                ss << "Data address and size: " << hex::IntToHex32(sectionInfo_[i].DataAddr);
                ss << "-" << hex::IntToHex32(sectionInfo_[i].DataAddr + sectionInfo_[i].DataSize - 1);
                ss << ", " << sectionInfo_[i].DataSize << " bytes" << std::endl;
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
                        ss << "Section items: " << (sectionInfo_[i].SubIdx2 - sectionInfo_[i].SubIdx1) << std::endl;
                        break;
                }
            }
            if (sectionInfo_[i].ParseStatus == 0)
            {
                ss << "!!! SECTION PARSE ERROR !!!" << std::endl;
            }
            if (sectionInfo_[i].ParseStatus == 2)
            {
                ss << "Section parse is not implemented" << std::endl;
            }

            if (infoRaw)
            {
                printRaw(ss, sectionInfo_[i].Addr, sectionInfo_[i].Size);
            }

            if (infoItem || infoCode || infoItemRaw)
            {
                switch (sectionInfo_[i].Id)
                {
                    case 1: printType(ss, sectionInfo_[i], infoItem, infoItemRaw, infoCode); break;
                    case 2: printImport(ss, sectionInfo_[i], infoItem, infoItemRaw, infoCode); break;
                    case 3: printFunction(ss, sectionInfo_[i], infoItem, infoItemRaw, infoCode); break;
                    case 4: printTable(ss, sectionInfo_[i], infoItem, infoItemRaw, infoCode); break;
                    case 5: printMemory(ss, sectionInfo_[i], infoItem, infoItemRaw, infoCode); break;
                    case 6: printGlobal(ss, sectionInfo_[i], infoItem, infoItemRaw, infoCode, decompType, decompBranch); break;
                    case 7: printExport(ss, sectionInfo_[i], infoItem, infoItemRaw, infoCode); break;
                    case 8: printStart(ss, sectionInfo_[i], infoItem, infoItemRaw, infoCode); break;
                    case 9: printElement(ss, sectionInfo_[i], infoItem, infoItemRaw, infoCode); break;
                    case 10: printCode(ss, sectionInfo_[i], infoItem, infoItemRaw, infoCode, decompType, decompBranch); break;
                    case 11: printData(ss, sectionInfo_[i], infoItem, infoItemRaw, infoCode, decompType, decompBranch); break;
                    case 12: printDataCount(ss, sectionInfo_[i], infoItem, infoItemRaw, infoCode); break;
                    case 13: printTag(ss, sectionInfo_[i], infoItem, infoItemRaw, infoCode); break;
                }
            }
        }
    }
    return ss.str();
}
