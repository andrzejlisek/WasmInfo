#include "filestructure.h"

bool fileStructure::isCodeGood(sectionSubInfo &sectionSubInfo__)
{
    if (sectionSubInfo__._CodeInstr.size() > 0)
    {
        if (sectionSubInfo__._CodeInstr[sectionSubInfo__._CodeInstr.size() - 1].Opcode < 0)
        {
            return false;
        }
        if (!sectionSubInfo__._CodeInstr[sectionSubInfo__._CodeInstr.size() - 1].errorMsg.empty())
        {
            return false;
        }
    }
    return true;
}

std::string fileStructure::valueTypeNameEx(int valueType)
{
    bool isNum = false;
    if (valueType < 0) { valueType = 0 - valueType; isNum = true; }
    if (!wasmDecompiler_.valueTypeIsStandard(valueType))
    {
        for (int i = 0; i < sectionInfo_.size(); i++)
        {
            if (sectionInfo_[i].Id == 1)
            {
                for (int ii = sectionInfo_[i].SubIdx1; ii < sectionInfo_[i].SubIdx2; ii++)
                {
                    if (sectionSubInfo_[ii].Index == valueType)
                    {
                        std::string s = "";
                        if (isNum)
                        {
                            s = std::to_string(sectionSubInfo_[ii]._TypeParams.size()) + "|" + std::to_string(sectionSubInfo_[ii]._TypeReturn.size());
                        }
                        else
                        {
                            for (int iii = 0; iii < sectionSubInfo_[ii]._TypeParams.size(); iii++)
                            {
                                if (iii > 0) s = s + "_";
                                s = s + wasmDecompiler_.valueTypeName(sectionSubInfo_[ii]._TypeParams[iii]);
                            }
                            if (sectionSubInfo_[ii]._TypeParams.size() == 0) s = s + "void";
                            s = s + "|";
                            for (int iii = 0; iii < sectionSubInfo_[ii]._TypeReturn.size(); iii++)
                            {
                                if (iii > 0) s = s + "_";
                                s = s + wasmDecompiler_.valueTypeName(sectionSubInfo_[ii]._TypeReturn[iii]);
                            }
                            if (sectionSubInfo_[ii]._TypeReturn.size() == 0) s = s + "void";
                        }
                        return s;
                    }
                }
            }
        }
        if (isNum)
        {
            return "0|0";
        }
        else
        {
            return "void|void";
        }
    }
    else
    {
        if (isNum)
        {
            if (valueType == 0x40)
            {
                return "0|0";
            }
            else
            {
                return "0|1";
            }
        }
        else
        {
            return wasmDecompiler_.valueTypeName(0x40) + "|" + wasmDecompiler_.valueTypeName(valueType);
        }
    }
}

std::string fileStructure::instructionText(codeInstr codeInstr_, int fidx)
{
    std::string text = "";

    if (codeInstr_.Opcode < 0) text = "!!! ERROR !!!";

    switch (codeInstr_.Opcode >> 8)
    {
        case 0x00: text = wasmDecompiler_.codeDef_[0][codeInstr_.Opcode & 255].nameAsm; break;
        case 0xFB: text = wasmDecompiler_.codeDef_[1][codeInstr_.Opcode & 255].nameAsm; break;
        case 0xFC: text = wasmDecompiler_.codeDef_[2][codeInstr_.Opcode & 255].nameAsm; break;
        case 0xFD: text = wasmDecompiler_.codeDef_[3][codeInstr_.Opcode & 255].nameAsm; break;
        case 0xFE: text = wasmDecompiler_.codeDef_[4][codeInstr_.Opcode & 255].nameAsm; break;
        case 0xFF: text = wasmDecompiler_.codeDef_[5][codeInstr_.Opcode & 255].nameAsm; break;
    }

    text = hex::StringFindReplace(text, "[~c~]", wasmDecompiler_.dataFieldDictionaryDisplay(codeInstr_.Param0));
    text = hex::StringFindReplace(text, "[~l~]", wasmDecompiler_.dataFieldDictionaryDisplay(wasmDecompiler_.dataFieldDictionaryGetVar("local", atoi(codeInstr_.Param0.c_str()))));
    text = hex::StringFindReplace(text, "[~g~]", wasmDecompiler_.dataFieldDictionaryDisplay(wasmDecompiler_.dataFieldDictionaryGetVar("global", atoi(codeInstr_.Param0.c_str()))));

    text = hex::StringFindReplace(text, "[~0~]", codeInstr_.Param0);
    text = hex::StringFindReplace(text, "[~1~]", codeInstr_.Param1);
    text = hex::StringFindReplace(text, "[~2~]", codeInstr_.Param2);
    if (hex::StringIndexOf(text, "[#0#]") > 0)
    {
        text = hex::StringFindReplace(text, "[#0#]", getFunctionNameById(getFunctionNameByIdMode::funcCode, getFunctionNameByIdNumber::whole, atoi(codeInstr_.Param0.c_str())));
    }
    if (hex::StringIndexOf(text, "[@0@]") > 0)
    {
        text = hex::StringFindReplace(text, "[@0@]", getFunctionNameById(getFunctionNameByIdMode::funcCode, getFunctionNameByIdNumber::type, atoi(codeInstr_.Param0.c_str())));
    }

    if ((hex::StringIndexOf(text, "[$0g]") > 0) || (hex::StringIndexOf(text, "[$0gg]") > 0))
    {
        text = hex::StringFindReplace(text, "[$0g]", wasmDecompiler_.valueTypeName(getVarTypeG(atoi(codeInstr_.Param0.c_str()))));
        text = hex::StringFindReplace(text, "[$0gg]", wasmDecompiler_.valueTypeName(getVarTypeG(atoi(codeInstr_.Param0.c_str()))));
    }

    if ((hex::StringIndexOf(text, "[$0l]") > 0) || (hex::StringIndexOf(text, "[$0ll]") > 0))
    {
        text = hex::StringFindReplace(text, "[$0l]", wasmDecompiler_.valueTypeName(getVarTypeL(atoi(codeInstr_.Param0.c_str()), fidx)));
        text = hex::StringFindReplace(text, "[$0ll]", wasmDecompiler_.valueTypeName(getVarTypeL(atoi(codeInstr_.Param0.c_str()), fidx)));
    }

    text = hex::StringFindReplace(text, "[\\n]", "\n");



    return text;
}

void fileStructure::loadNames(int setType, std::string nameText)
{
    int ptr = 0;
    for (int i = 0; i < 256; i++)
    {
        wasmDecompiler_.codeDef_[setType][i].nameAsm = "";
        wasmDecompiler_.codeDef_[setType][i].paramAsm = "";
        wasmDecompiler_.codeDef_[setType][i].stackParam = "";
        wasmDecompiler_.codeDef_[setType][i].stackResult = "";
        wasmDecompiler_.codeDef_[setType][i].nameDecomp = "";
        wasmDecompiler_.codeDef_[setType][i].stackResultType = -1;
    }

    int lineItem = 0;
    for (int i = 0; i < nameText.size(); i++)
    {
        bool std = true;
        if (nameText[i] == '\r')
        {
            std = false;
        }
        if (nameText[i] == '\n')
        {
            std = false;
            ptr++;
            lineItem = 0;
        }
        if (nameText[i] == '`')
        {
            std = false;
            lineItem++;
        }
        if (std)
        {
            switch (lineItem)
            {
                case 0:
                    wasmDecompiler_.codeDef_[setType][ptr].paramAsm.push_back(nameText[i]);
                    break;
                case 1:
                    wasmDecompiler_.codeDef_[setType][ptr].nameAsm.push_back(nameText[i]);
                    break;
                case 2:
                    wasmDecompiler_.codeDef_[setType][ptr].stackParam.push_back(nameText[i]);
                    break;
                case 3:
                    wasmDecompiler_.codeDef_[setType][ptr].stackResult.push_back(nameText[i]);
                    break;
                case 4:
                    wasmDecompiler_.codeDef_[setType][ptr].nameDecomp.push_back(nameText[i]);
                    break;
            }
        }
    }

    for (int i = 0; i < 256; i++)
    {
        wasmDecompiler_.codeDef_[setType][i].stackResultType = -1;
        if (wasmDecompiler_.codeDef_[setType][i].stackResult == "par0")
        {
            wasmDecompiler_.codeDef_[setType][i].stackResultType = wasmDecompiler::fieldTypeVariable;
            wasmDecompiler_.codeDef_[setType][i].stackResult = "1";
        }
        if (wasmDecompiler_.codeDef_[setType][i].stackResult == "i32")
        {
            wasmDecompiler_.codeDef_[setType][i].stackResultType = wasmDecompiler::fieldType_i32;
            wasmDecompiler_.codeDef_[setType][i].stackResult = "1";
        }
        if (wasmDecompiler_.codeDef_[setType][i].stackResult == "i64")
        {
            wasmDecompiler_.codeDef_[setType][i].stackResultType = wasmDecompiler::fieldType_i64;
            wasmDecompiler_.codeDef_[setType][i].stackResult = "1";
        }
        if (wasmDecompiler_.codeDef_[setType][i].stackResult == "f32")
        {
            wasmDecompiler_.codeDef_[setType][i].stackResultType = wasmDecompiler::fieldType_f32;
            wasmDecompiler_.codeDef_[setType][i].stackResult = "1";
        }
        if (wasmDecompiler_.codeDef_[setType][i].stackResult == "f64")
        {
            wasmDecompiler_.codeDef_[setType][i].stackResultType = wasmDecompiler::fieldType_f64;
            wasmDecompiler_.codeDef_[setType][i].stackResult = "1";
        }
        if (wasmDecompiler_.codeDef_[setType][i].stackResult == "v128")
        {
            wasmDecompiler_.codeDef_[setType][i].stackResultType = wasmDecompiler::fieldType_v128;
            wasmDecompiler_.codeDef_[setType][i].stackResult = "1";
        }
        if (wasmDecompiler_.codeDef_[setType][i].stackResult == "anyref")
        {
            wasmDecompiler_.codeDef_[setType][i].stackResultType = wasmDecompiler::fieldType_anyref;
            wasmDecompiler_.codeDef_[setType][i].stackResult = "1";
        }
        if (wasmDecompiler_.codeDef_[setType][i].stackResult == "funcref")
        {
            wasmDecompiler_.codeDef_[setType][i].stackResultType = wasmDecompiler::fieldType_funcref;
            wasmDecompiler_.codeDef_[setType][i].stackResult = "1";
        }
        if (wasmDecompiler_.codeDef_[setType][i].stackResult == "externref")
        {
            wasmDecompiler_.codeDef_[setType][i].stackResultType = wasmDecompiler::fieldType_externref;
            wasmDecompiler_.codeDef_[setType][i].stackResult = "1";
        }
        if (wasmDecompiler_.codeDef_[setType][i].stackResult == "exnref")
        {
            wasmDecompiler_.codeDef_[setType][i].stackResultType = wasmDecompiler::fieldType_exnref;
            wasmDecompiler_.codeDef_[setType][i].stackResult = "1";
        }
        if (wasmDecompiler_.codeDef_[setType][i].stackResult == "ignore")
        {
            wasmDecompiler_.codeDef_[setType][i].stackResultType = wasmDecompiler::fieldTypeIgnore;
            wasmDecompiler_.codeDef_[setType][i].stackResult = "0";
        }
    }
}
