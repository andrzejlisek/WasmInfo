#include "wasmdecompiler.h"

void wasmDecompiler::currentStackPush(int valType)
{
    if (currentStack_.size() > 0)
    {
        if (currentStack_[currentStack_.size() - 1] == 0x40)
        {
            return;
        }
    }
    currentStack_.push_back(valType);
}

bool wasmDecompiler::currentStackPop()
{
    if (currentStack_.size() > 0)
    {
        if (currentStack_[currentStack_.size() - 1] < 0)
        {
            return false;
        }
        else
        {
            if (currentStack_[currentStack_.size() - 1] != 0x40)
            {
                currentStack_.pop_back();
            }
            return true;
        }
    }
    else
    {
        return false;
    }
}

void wasmDecompiler::currentStackIgnore()
{
    while (currentStack_[currentStack_.size() - 1] >= 0)
    {
        currentStack_.pop_back();
    }
    std::string stackR_ = currentStackStackR_[currentStackStackR_.size() - 1];
    int stackR = currentStackStackR[currentStackStackR.size() - 1];
    for (int i = 0; i < stackR; i++)
    {
        currentStack_.push_back(valueTypeNumber(hex::StringGetParam(stackR_, i, '_').c_str()));
    }

    currentStack_.push_back(0x40);
}

int wasmDecompiler::currentStackSize(int sizeType)
{
    int X = 0;
    for (int i = 0; i < currentStack_.size(); i++)
    {
        if (currentStack_[i] >= 0)
        {
            if (currentStack_[i] == fieldType_void)
            {
                return -1;
            }
            X++;
        }
        else
        {
            if (sizeType == 1)
            {
                X = 0;
            }
        }
    }
    return X;
}

std::string wasmDecompiler::currentStackPrint()
{
    std::string X = "";
    bool wasPrint = false;
    for (int i = 0; i < currentStack_.size(); i++)
    {
        if (currentStack_[i] >= 0)
        {
            if (wasPrint) X = X + "_";
            X = X + valueTypeName(currentStack_[i]);
            wasPrint = true;
        }
        else
        {
            X = X + "|";
            wasPrint = false;
        }
    }
    return X;
}

void wasmDecompiler::currentStackBlockPrepare(int stackP, int stackR, std::string stackP_, std::string stackR_)
{
    currentStack_.clear();
    currentStack_.push_back(-1);
    if (stackR_ == "void_")
    {
        stackR_ = "";
        currentStack_.push_back(0x40);
    }

    currentStackStackP.clear();
    currentStackStackR.clear();
    currentStackStackP_.clear();
    currentStackStackR_.clear();
    currentStackStackP.push_back(stackP);
    currentStackStackR.push_back(stackR);
    currentStackStackP_.push_back(stackP_);
    currentStackStackR_.push_back(stackR_);
}

void wasmDecompiler::currentStackBlockRestore(bool clearParams)
{
    std::string stackS = currentStackStackP_[currentStackStackP_.size() - 1];
    int stackP = currentStackStackP[currentStackStackP.size() - 1];
    while (currentStack_[currentStack_.size() - 1] > 0)
    {
        currentStack_.pop_back();
    }
    if (!clearParams)
    {
        for (int i = 0; i < stackP; i++)
        {
            currentStack_.push_back(valueTypeNumber(hex::StringGetParam(stackS, i, '_').c_str()));
        }
    }
}

bool wasmDecompiler::currentStackBlockPush(int stackP, int stackR, std::string stackP_, std::string stackR_)
{
    if (stackP_ == "void")
    {
        stackP_ = "";
    }
    else
    {
        stackP_ = stackP_ + "_";
    }

    if (stackR_ == "void")
    {
        stackR_ = "";
    }
    else
    {
        stackR_ = stackR_ + "_";
    }

    std::string stackS = "";
    currentStack_.push_back(-1);
    if (stackP > 0)
    {
        for (int i = 0; i < stackP; i++)
        {
            stackS = stackS + valueTypeName(currentStack_[currentStack_.size() - 1 - stackP + i]) + "_";
        }
        for (int i = 0; i < stackP; i++)
        {
            currentStack_[currentStack_.size() - 1 - i] = currentStack_[currentStack_.size() - 2 - i];
        }
        currentStack_[currentStack_.size() - 1 - stackP] = -1;
    }

    currentStackStackP.push_back(stackP);
    currentStackStackR.push_back(stackR);
    currentStackStackP_.push_back(stackP_);
    currentStackStackR_.push_back(stackR_);

    if (stackP_ == stackS)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool wasmDecompiler::currentStackBlockPop()
{
    int sepIdx = -1;
    for (int i = 0; i < currentStack_.size(); i++)
    {
        if (currentStack_[i] < 0)
        {
            sepIdx = i;
        }
    }
    if (sepIdx < 0) return false;

    int stackR = currentStackStackR[currentStackStackR.size() - 1];

    if (currentStack_[currentStack_.size() - 1] == 0x40)
    {
        currentStack_.pop_back();
    }

    if (currentStack_.size() <= stackR)
    {
        return false;
    }

    if (stackR > 0)
    {
        for (int i = 0; i < stackR; i++)
        {
            currentStack_[sepIdx + i] = currentStack_[sepIdx + i + 1];
        }
        currentStack_[sepIdx + stackR] = -1;
    }

    currentStackStackP.pop_back();
    currentStackStackR.pop_back();
    currentStackStackP_.pop_back();
    currentStackStackR_.pop_back();
    if (currentStack_[currentStack_.size() - 1] >= 0)
    {
        return false;
    }
    else
    {
        currentStack_.pop_back();
        return true;
    }
}

std::string wasmDecompiler::stackPrintInfo(std::string stackInfo, int stackDepth)
{
    std::string stackStr;

    if (stackInfo.size() >= 2)
    {
        for (int i = 0; i < (stackInfo.size() - 1); i++)
        {
            if ((stackInfo[i] == '|') && (stackInfo[i + 1] != '|'))
            {
                stackInfo = stackInfo.substr(0, i + 1) + "_" + stackInfo.substr(i + 1);
                i++;
            }
        }
    }
    if ((!stackInfo.empty()) && (stackInfo[0] != '_') && (stackInfo[0] != '|'))
    {
        stackInfo = "_" + stackInfo;
    }

    if ((stackInfo.size() >= 5) && (stackInfo.substr(stackInfo.size() - 5) == "_void"))
    {
        int t = stackInfo.size() - 5;
        stackStr = stackInfo.substr(0, t) + "~";
    }
    else
    {
        if (stackDepth > 0)
        {
            int stackDepth_ = stackDepth;
            int usageSeparator = stackInfo.size();
            while ((usageSeparator > 0) && (stackDepth_ > 0))
            {
                usageSeparator--;
                if (stackInfo[usageSeparator] == '_')
                {
                    stackDepth_--;
                }

                if (stackDepth_ == 0)
                {
                    usageSeparator++;
                    break;
                }
            }
            if (usageSeparator > 0)
            {
                stackStr = stackInfo.substr(0, usageSeparator - 1) + "*" + stackInfo.substr(usageSeparator);
            }
            else
            {
                stackStr = "!" + stackInfo;
            }
        }
        else
        {
            stackStr = stackInfo + "*";
        }
    }

    int stackStrSize = stackStr.size();
    if (codeInstrInfoLength < (stackStrSize + 3))
    {
        codeInstrInfoLength = (stackStrSize + 3);
    }
    return stackStr + hex::pad(codeInstrInfoLength - stackStrSize);
}

std::string wasmDecompiler::stackPrintInfoFull(int stackSize, std::string stackInfoI, int stackDepthI, std::string stackInfoO, int stackDepthO)
{
    return stackPrintInfo(stackInfoI, stackDepthI) + stackPrintInfo(stackInfoO, stackDepthO);
}

std::string wasmDecompiler::stackPrintInfoBlank()
{
    return hex::pad(codeInstrInfoLength) + hex::pad(codeInstrInfoLength);
}
