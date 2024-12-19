#include "wasmdecompilerfunction.h"

wasmDecompilerFunction::wasmDecompilerFunction()
{

}

void wasmDecompilerFunction::paramAdd(std::string text)
{
    std::shared_ptr<wasmDecompilerFunction> instr = std::make_shared<wasmDecompilerFunction>();
    instr.get()->name = text;
    instr.get()->isFoldable = true;
    params.push_back(instr);
}

void wasmDecompilerFunction::additionalInstr(int currentDepth, std::string instrResult, std::string instrParam, int instrId, std::string comment)
{
    std::shared_ptr<wasmDecompilerFunction> instr = std::make_shared<wasmDecompilerFunction>();
    instr.get()->blockFold = false;
    instr.get()->isFoldable = (instrResult != "");
    instr.get()->depth = currentDepth;
    if (instrResult != "")
    {
        instr.get()->printComma = true;
        instr.get()->name = "[~0~]";
        instr.get()->returnName = instrResult;
        instr.get()->returnNameItems.push_back(instrResult);
        instr.get()->paramAdd(instrParam);
    }
    else
    {
        instr.get()->name = instrParam;
        instr.get()->returnName = "";
    }
    instr.get()->comment = comment;
    instr.get()->id = instrId;
    params.push_back(instr);
}

std::string wasmDecompilerFunction::instrText()
{
    std::string s = name;
    for (int i = 0; i < params.size(); i++)
    {
        s = hex::StringFindReplace(s, "[~" + std::to_string(i) + "~]", params[i].get()->instrText());
    }
    if (hex::StringIndexOf(s, "[#1#]") >= 0)
    {
        std::string paramList = "";
        for (int i = instrTextParamList; i < params.size(); i++)
        {
            if (i > instrTextParamList)
            {
                paramList = paramList + ", ";
            }
            paramList = paramList + params[i].get()->instrText();
        }
        s = hex::StringFindReplace(s, "[#1#]", paramList);
    }
    return s;
}
