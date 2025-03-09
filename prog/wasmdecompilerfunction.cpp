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

void wasmDecompilerFunction::additionalInstr(int currentDepth, std::string instrResult, std::string instrParam, int instrId)
{
    std::shared_ptr<wasmDecompilerFunction> instr = std::make_shared<wasmDecompilerFunction>();
    instr.get()->blockFold = false;
    instr.get()->isFoldable = (instrResult != "");
    instr.get()->depth = currentDepth;
    if (instrResult != "")
    {
        if (instrResult[0] == '.')
        {
            instr.get()->isFoldable = false;
            instrResult = instrResult.substr(1);
        }

        instr.get()->printComma = true;
        instr.get()->name = "[~0~]";
        instr.get()->returnName = instrResult;
        instr.get()->returnType = 255;
        instr.get()->returnNameItems.push_back(instrResult);
        instr.get()->paramAdd(instrParam);
    }
    else
    {
        instr.get()->name = instrParam;
        instr.get()->returnName = "";
    }
    instr.get()->id = instrId;
    params.push_back(instr);
}

std::string wasmDecompilerFunction::instrText()
{
    std::string s = name;
    for (int i = 0; i < params.size(); i++)
    {
        std::string paramsText = params[i].get()->instrText();
        if ((paramsText.size() > 2) && (paramsText[0] == '(') && (paramsText[paramsText.size() - 1] == ')'))
        {
            s = hex::StringFindReplace(s, "([~" + std::to_string(i) + "~])", paramsText);
        }
        s = hex::StringFindReplace(s, "[~" + std::to_string(i) + "~]", paramsText);
        if (hex::StringIndexOf(s, "[!" + std::to_string(i) + "!]") >= 0)
        {
            std::string repl = params[i].get()->instrText();
            s = hex::StringFindReplace(s, "[!" + std::to_string(i) + "!]", repl);
        }
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
        if ((paramList.size() > 2) && (paramList[0] == '(') && (paramList[paramList.size() - 1] == ')'))
        {
            s = hex::StringFindReplace(s, "([#1#])", paramList);
        }
        s = hex::StringFindReplace(s, "[#1#]", paramList);
    }
    return s;
}
