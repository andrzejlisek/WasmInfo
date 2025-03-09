#include "wasmdecompiler.h"

void wasmDecompiler::addCommand(unsigned char * raw, int addr, int size, std::string par0, std::string par1, std::string par2)
{
    // Adding final "end" command requires adding additional "return" command when the function returns value and the stack is not empty
    if ((currentDepth == 1) && (raw[addr] == 0x0B))
    {
        if (currentStack > 0)
        {
            unsigned char * dummyRaw = new unsigned char[4];
            dummyRaw[0] = 0x0F;
            dummyRaw[1] = 0x00;
            dummyRaw[2] = 0x00;
            dummyRaw[3] = 0x00;
            switch (lastOpcode)
            {
                case 0x0F: // return
                case 0x12: // return_call
                case 0x13: // return_call_indirect
                case 0x15: // return_call_ref
                    break;
                default:
                    break;
            }
            addCommand(dummyRaw, 0, 1, "", "", "");
            delete[] dummyRaw;
        }
    }


    // Create dummy instructions containing the function name
    if (WDF.params.size() == 0)
    {
        if (funcName != "")
        {
            std::string s = "";
            int returnCount = 0;
            for (int i = 0; i < dataFieldDictionary.size(); i++)
            {
                if (dataFieldDictionary[i].fieldCategory == "return")
                {
                    if (returnCount > 0)
                    {
                        s = s + "_";
                    }
                    s = s + valueTypeName(dataFieldDictionary[i].fieldType);
                    returnCount++;
                }
            }
            if (returnCount == 0) s = "void";
            s = s + " " + funcName + "(";
            int paramCount = 0;
            for (int i = 0; i < dataFieldDictionary.size(); i++)
            {
                if ((dataFieldDictionary[i].fieldCategory == "local") && (dataFieldDictionary[i].isParam))
                {
                    if (paramCount > 0)
                    {
                        s = s + ", ";
                    }
                    s = s + valueTypeName(dataFieldDictionary[i].fieldType) + " " + dataFieldDictionaryDisplay(dataFieldDictionary[i].fieldId);
                    paramCount++;
                }
            }
            if (paramCount == 0) s = s + "void";
            s = s + ")";
            WDF.additionalInstr(0, "", s, 0);
        }
        WDF.additionalInstr(0, "", "{", 0);
    }


    int instrId = WDF.params.size();

    std::shared_ptr<wasmDecompilerFunction> Instr = std::make_shared<wasmDecompilerFunction>();
    Instr.get()->id = instrId;
    int opCode = raw[addr];
    int nameTbl = 0;
    switch (opCode)
    {
        case 0xFB: nameTbl = 1; opCode = (opCode << 8) + (int)(raw[addr + 1]); break;
        case 0xFC: nameTbl = 2; opCode = (opCode << 8) + (int)(raw[addr + 1]); break;
        case 0xFD: nameTbl = 3; opCode = (opCode << 8) + (int)(raw[addr + 1]); break;
        case 0xFE: nameTbl = 4; opCode = (opCode << 8) + (int)(raw[addr + 1]); break;
    }

    // Distinguish the SIMD planes
    if ((nameTbl == 3) && (raw[addr + 1] >= 0x80))
    {
        if (raw[addr + 2] == 0x02)
        {
            opCode = opCode + 0x0200;
            nameTbl = 5;
        }
    }

    lastOpcode = opCode;
    Instr.get()->name = codeDef_[nameTbl][opCode & 255].nameDecomp;

    int stackP = atoi(codeDef_[nameTbl][opCode & 255].stackParam.c_str());
    int stackR = atoi(codeDef_[nameTbl][opCode & 255].stackResult.c_str());
    std::string codeParamStr = codeDef_[nameTbl][opCode & 255].paramAsm;
    Instr.get()->returnType = codeDef_[nameTbl][opCode & 255].stackResultType;

    bool stackSimplified = decompOptStackSimplify;


    if (codeParamStr == "t")
    {
        int stackP_ = 0;
        int stackR_ = 0;
        if (hex::StringGetParam(par1 + "||", 1, '|') == "")
        {
            stackP_ = atoi(hex::StringGetParam(par1 + "||", 0, '|').c_str());
        }
        else
        {
            stackP_ = atoi(hex::StringGetParam(par1 + "||", 0, '|').c_str());
            stackR_ = atoi(hex::StringGetParam(par1 + "||", 1, '|').c_str());
        }
        stackP = stackP_;
        //stackR = stackR_;
    }

    switch (opCode)
    {
        case 0x02: // block
            stackSimplified = false;
            Instr.get()->branchType = 1;
            break;
        case 0x04: // if
        case 0x05: // else
        case 0x06: // try
        case 0x07: // catch
        case 0x19: // catch_all
            if ((codeParamStr == "u") || (codeParamStr == "b") || (codeParamStr == "16b") || (codeParamStr == "uxu"))
            {
                Instr.get()->paramAdd(par0);
            }
            stackSimplified = false;
            Instr.get()->branchType = 2;
            break;
        case 0x03: // loop
            stackSimplified = false;
            Instr.get()->branchType = 3;
            break;
        case 0x0B: // end
            stackSimplified = false;
            Instr.get()->branchType = 4;
            break;
        case 0x10: // call
        case 0x11: // call_indirect
        case 0x12: // return_call
        case 0x13: // return_call_indirect
        case 0x14: // call_ref
        case 0x15: // return_call_ref
            {
                std::string parStackP = hex::StringGetParam(par2, 0, '`');
                std::string parStackR = hex::StringGetParam(par2, 1, '`');
                std::string parFuncName = hex::StringGetParam(par2, 2, '`');

                stackP = atoi(parStackP.c_str());
                stackR = atoi(parStackR.c_str());

                if ((opCode == 0x11) || (opCode == 0x13))
                {
                    stackP++;
                    Instr.get()->paramAdd(par0);
                    Instr.get()->paramAdd(par1);
                    Instr.get()->instrTextParamList = 2;
                }

                Instr.get()->name = hex::StringFindReplace(Instr.get()->name, "[#0#]", parFuncName);
                switch (stackR)
                {
                    case 0:
                        Instr.get()->returnType = 0;
                        break;
                    case 1:
                        Instr.get()->returnType = std::atoi(hex::StringGetParam(par2, 3, '`').c_str());
                        break;
                    default:
                        Instr.get()->returnType = fieldTypeCallFunc;
                        Instr.get()->returnTypeList.clear();
                        for (int i = 0; i < stackR; i++)
                        {
                            Instr.get()->returnTypeList.push_back(std::atoi(hex::StringGetParam(par2, 3 + i, '`').c_str()));
                        }
                        break;
                }
            }
            break;
        case 0x0F: // return
            {
                stackP = currentStack;
            }
            break;
        case 0x20: // local.get
        case 0x21: // local.set
        case 0x22: // local.tee
        case 0x23: // global.get
        case 0x24: // global.set
        case 0x0C: // br
        case 0x0D: // br_if
            break;
        default:
            {
                if ((codeParamStr == "i32") || (codeParamStr == "i64") || (codeParamStr == "f32") || (codeParamStr == "f64"))
                {
                    Instr.get()->paramAdd(par0);
                }
                if ((codeParamStr == "u") || (codeParamStr == "b") || (codeParamStr == "16b") || (codeParamStr == "uxu"))
                {
                    Instr.get()->paramAdd(par0);
                }
                if (codeParamStr == "uu")
                {
                    Instr.get()->paramAdd(par0);
                    Instr.get()->paramAdd(par1);
                }
                if (codeParamStr == "uub")
                {
                    Instr.get()->paramAdd(par0);
                    Instr.get()->paramAdd(par1);
                    Instr.get()->paramAdd(par2);
                }
            }
            break;
    }


    if (stackSimplified)
    {
        for (int i = 0; i < stackP; i++)
        {
            Instr.get()->paramAdd(dataFieldDictionaryGetVar("stack", stackP - 1 - i));
        }
    }
    else
    {
        for (int i = 0; i < stackP; i++)
        {
            Instr.get()->paramAdd(dataFieldDictionarySet("", 0, "tempp", i + tempVarCounterP));
        }
    }

    Instr.get()->blockFold = false;
    Instr.get()->printComma = true;

    switch (opCode)
    {
        case 0x0F: // return
        case 0x12: // return_call
        case 0x13: // return_call_indirect
        case 0x15: // return_call_ref
            break;
        case 0x02: // block
        case 0x03: // loop
            Instr.get()->paramAdd(par0);
            Instr.get()->printComma = false;
            Instr.get()->blockFold = true;
            break;
        case 0x04: // if
        case 0x05: // else
        case 0x06: // try
        case 0x07: // catch
        case 0x0B: // end
        case 0x19: // catch_all
            Instr.get()->printComma = false;
            Instr.get()->blockFold = true;
            break;
        case 0x0C: // br
        case 0x0D: // br_if
            Instr.get()->branchDepth = atoi(par0.c_str()) + 1;
            Instr.get()->paramAdd(par0);
            break;
        case 0x0E: // br_table
            Instr.get()->branchDepth = branchDepthMagicNum;
            break;
        case 0x20: // local.get
            Instr.get()->paramAdd(dataFieldDictionaryGetVar("local", atoi(par0.c_str())));
            Instr.get()->returnType = fieldTypeVariable;
            break;
        case 0x21: // local.set
            Instr.get()->returnName = dataFieldDictionarySet("", 0, "local", atoi(par0.c_str()));
            Instr.get()->returnType = fieldTypeVariable;
            break;
        case 0x22: // local.tee
            Instr.get()->returnName = dataFieldDictionarySet("", 0, "local", atoi(par0.c_str()));
            Instr.get()->returnType = fieldTypeVariable;
            Instr.get()->paramAdd(dataFieldDictionaryGetVar("stack", 0));
            break;
        case 0x23: // global.get
            Instr.get()->paramAdd(dataFieldDictionaryGetVar("global", atoi(par0.c_str())));
            Instr.get()->returnType = fieldTypeVariable;
            break;
        case 0x24: // global.set
            Instr.get()->returnName = dataFieldDictionarySet("", 0, "global", atoi(par0.c_str()));
            Instr.get()->returnType = fieldTypeVariable;
            break;
    }

    Instr.get()->isFoldable = false;
    if ((stackR == 1) && (Instr.get()->returnName == ""))
    {
        Instr.get()->returnName = dataFieldDictionarySet("", 0, "tempr", tempVarCounterR);
    }
    if (Instr.get()->returnName != "")
    {
        if (codeParamStr != "16b")
        {
            Instr.get()->isFoldable = true;
        }
    }

    Instr.get()->returnNameItems.clear();
    if (stackR > 1)
    {
        Instr.get()->returnName = "";
        for (int i = 0; i < stackR; i++)
        {
            if (i > 0) Instr.get()->returnName = Instr.get()->returnName + ", ";
            Instr.get()->returnName = Instr.get()->returnName + dataFieldDictionarySet("", 0, "tempr", i + tempVarCounterR);
            Instr.get()->returnNameItems.push_back(dataFieldDictionarySet("", 0, "tempr", i + tempVarCounterR));
        }
        Instr.get()->returnName = "{" + Instr.get()->returnName + "}";
    }
    else
    {
        if (Instr.get()->returnName != "")
        {
            Instr.get()->returnNameItems.push_back(Instr.get()->returnName);
        }
    }


    switch (opCode)
    {
        case 0x24: // global.set
            Instr.get()->isFoldable = false;
            break;
        case 0x05: // else
        case 0x07: // catch
        case 0x19: // catch_all
            currentDepth--;
            while (stackSizeBlock.size() <= (currentDepth))
            {
                stackSizeBlock.push_back(0);
                stackSizeIf.push_back(-1);
                stackSizeDiff.push_back(0);
            }
            stackSizeIf[currentDepth] = currentStack;
            currentStack = stackSizeBlock[currentDepth];
            break;
        case 0x0B: // end
            currentDepth--;
            while (stackSizeBlock.size() <= (currentDepth))
            {
                stackSizeBlock.push_back(0);
                stackSizeIf.push_back(-1);
                stackSizeDiff.push_back(0);
            }
            if (stackSizeIf[currentDepth] >= 0)
            {
                if (stackSizeIf[currentDepth] != currentStack)
                {
                    WDF.additionalInstr(0, "", "!!! STACK WARNING !!!", 0);
                }
            }
            else
            {
                if (currentStack != stackSizeBlock[currentDepth])
                {
                    WDF.additionalInstr(0, "", "!!! STACK WARNING !!!", 0);
                }
            }
            //currentStack = stackSizeBlock[currentDepth];
            break;
        case 0x0C: // br
        case 0x0E: // br_table
        case 0x0F: // return
        case 0x12: // return_call
        case 0x13: // return_call_indirect
        case 0x15: // return_call_ref
            {
                if (currentStack > stackSizeBlock[currentDepth - 1])
                {
                    stackP = currentStack - stackSizeBlock[currentDepth - 1];
                }
                if (stackSizeBlock[currentDepth - 1] > currentStack)
                {
                    stackR = stackSizeBlock[currentDepth - 1] - currentStack;
                }
            }
            break;
    }
    Instr.get()->depth = currentDepth;

    for (int i = 0; i < size; i++)
    {
        Instr.get()->originalInstr.push_back(raw[addr + i]);
    }

    int currentStack1 = currentStack;

    if (!stackSimplified)
    {
        // Copy parameters from stack to temporary variables
        for (int i = 0; i < stackP; i++)
        {
            WDF.additionalInstr(currentDepth, dataFieldDictionarySet("", dataFieldDictionaryGetType("stack", stackP - 1 - i), "tempp", i + tempVarCounterP), dataFieldDictionaryGetVar("stack", stackP - 1 - i), instrId);
        }

        // Remove parameters from the stack
        if (stackP > 0)
        {
            currentStack -= stackP;
            for (int i = 0; i < currentStack; i++)
            {
                WDF.additionalInstr(currentDepth, dataFieldDictionarySet("", dataFieldDictionaryGetType("stack", i + stackP), "stack", i), dataFieldDictionaryGetVar("stack", i + stackP), instrId);
            }
        }
    }

    // Execute instruction
    if (currentDepth > 0)
    {
        Instr.get()->stackPrint = true;
    }
    WDF.params.push_back(Instr);

    if (stackSimplified)
    {
        // More parameters are more than results ==> Remove stack items
        if (stackP > stackR)
        {
            currentStack -= (stackP - stackR);
            for (int i = stackR; i < currentStack; i++)
            {
                WDF.additionalInstr(currentDepth, dataFieldDictionarySet("", dataFieldDictionaryGetType("stack", i + stackP - stackR), "stack", i), dataFieldDictionaryGetVar("stack", i + stackP - stackR), instrId);
            }
        }

        // Less parameters are more than results ==> Add stack items
        if (stackP < stackR)
        {
            for (int i = currentStack - 1; i >= stackP; i--)
            {
                WDF.additionalInstr(currentDepth, dataFieldDictionarySet("", dataFieldDictionaryGetType("stack", i), "stack", i + stackR - stackP), dataFieldDictionaryGetVar("stack", i), instrId);
            }
            currentStack += (stackR - stackP);
        }
    }
    else
    {

        // Insert results to the stack
        if (stackR > 0)
        {
            for (int i = currentStack - 1; i >= 0; i--)
            {
                WDF.additionalInstr(currentDepth, dataFieldDictionarySet("", dataFieldDictionaryGetType("stack", i), "stack", i + stackR), dataFieldDictionaryGetVar("stack", i), instrId);
            }
            currentStack += stackR;
        }
    }

    // Copy result from temporary variables to stack
    for (int i = 0; i < stackR; i++)
    {
        WDF.additionalInstr(currentDepth, dataFieldDictionarySet("", dataFieldDictionaryGetType("tempr", i + tempVarCounterR), "stack", stackR - 1 - i), dataFieldDictionaryGetVar("tempr", i + tempVarCounterR), instrId);
    }

    int currentStack2 = currentStack;
    Instr.get()->stackP = stackP;
    Instr.get()->stackR = stackR;
    Instr.get()->stackI = currentStack1;
    Instr.get()->stackO = currentStack2;

    switch (opCode)
    {
        case 0x02: // block
        case 0x03: // loop
        case 0x04: // if
        case 0x06: // try
            while (stackSizeBlock.size() <= (currentDepth))
            {
                stackSizeBlock.push_back(0);
                stackSizeIf.push_back(-1);
                stackSizeDiff.push_back(0);
            }
            stackSizeBlock[currentDepth] = currentStack;
            stackSizeIf[currentDepth] = -1;
            currentDepth++;
            break;
        case 0x05: // else
        case 0x07: // catch
        case 0x19: // catch_all
            while (stackSizeBlock.size() <= (currentDepth))
            {
                stackSizeBlock.push_back(0);
                stackSizeIf.push_back(-1);
                stackSizeDiff.push_back(0);
            }
            stackSizeBlock[currentDepth] = currentStack;
            currentDepth++;
            break;
    }

    tempVarCounterP = tempVarCounterP + stackP;
    tempVarCounterR = tempVarCounterR + stackR;
}
