#include "wasmdecompiler.h"

void wasmDecompiler::addCommandStackDummy(int valueType)
{
    int base = dummyRaw.size();
    dummyRaw.push_back(0);
    dummyRaw.push_back(0);
    dummyRaw.push_back(0);
    dummyRaw.push_back(0);
    dummyRaw.push_back(0);
    dummyRaw.push_back(0);
    dummyRaw.push_back(0);
    dummyRaw.push_back(0);
    dummyRaw.push_back(0);
    dummyRaw.push_back(0);
    int raw0Size = 1;
    std::string raw0Par0 = "";

    switch (valueType)
    {
        case fieldType_i32:
            dummyRaw[base + 0] = 0x41;
            raw0Size = 2;
            raw0Par0 = dataFieldDictionarySet("0", fieldType_i32, "const", 0);
            break;
        case fieldType_i64:
            dummyRaw[base + 0] = 0x42;
            raw0Size = 2;
            raw0Par0 = dataFieldDictionarySet("0", fieldType_i64, "const", 0);
            break;
        case fieldType_f32:
            dummyRaw[base + 0] = 0x43;
            raw0Size = 5;
            raw0Par0 = dataFieldDictionarySet("0.0", fieldType_f32, "const", 0);
            break;
        case fieldType_f64:
            dummyRaw[base + 0] = 0x44;
            raw0Size = 9;
            raw0Par0 = dataFieldDictionarySet("0.0", fieldType_f64, "const", 0);
            break;
        case fieldType_v128:
            dummyRaw[base + 0] = 0xFD;
            dummyRaw[base + 1] = 0x0C;
            raw0Size = 10;
            raw0Par0 = "0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0";
            break;
        case -1: // Simulate "drop" to remove from stack
            dummyRaw[base + 0] = 0x1A;
            raw0Size = 1;
            raw0Par0 = "";
            break;
        case -2: // Simulate "end"
            dummyRaw[base + 0] = 0x0B;
            raw0Size = 1;
            raw0Par0 = "";
            break;
        case -3: // Simulate "return"
            dummyRaw[base + 0] = 0x0F;
            raw0Size = 1;
            raw0Par0 = "";
            break;
    }

    addCommand(dummyRaw.data(), base, raw0Size, raw0Par0, "", "", "x", "x", 0, 0, 0);
    dummyRaw.pop_back();
    dummyRaw.pop_back();
    dummyRaw.pop_back();
    dummyRaw.pop_back();
    dummyRaw.pop_back();
    dummyRaw.pop_back();
    dummyRaw.pop_back();
    dummyRaw.pop_back();
    dummyRaw.pop_back();
    dummyRaw.pop_back();
}

void wasmDecompiler::addCommand(std::string instr, std::string par0, std::string par1, std::string par2, std::string stackI__, std::string stackO__, int stackP__, int stackR__, int stackS__)
{
    int base = dummyRaw.size();
    dummyRaw.push_back(0);
    dummyRaw.push_back(0);
    dummyRaw.push_back(0);
    dummyRaw.push_back(0);
    dummyRaw.push_back(0);
    dummyRaw.push_back(0);
    dummyRaw.push_back(0);
    dummyRaw.push_back(0);
    dummyRaw.push_back(0);
    dummyRaw.push_back(0);
    int raw0Size = instr.size() / 2;
    for (int i = 0; i < raw0Size; i++)
    {
        dummyRaw[base + i] = hex::HexToInt(instr.substr(i << 1, 2));
    }
    addCommand(dummyRaw.data(), base, raw0Size, par0, par1, par2, stackI__, stackO__, stackP__, stackR__, stackS__);
    dummyRaw.pop_back();
    dummyRaw.pop_back();
    dummyRaw.pop_back();
    dummyRaw.pop_back();
    dummyRaw.pop_back();
    dummyRaw.pop_back();
    dummyRaw.pop_back();
    dummyRaw.pop_back();
    dummyRaw.pop_back();
    dummyRaw.pop_back();
}

void wasmDecompiler::addCommand(unsigned char * raw, int addr, int size, std::string par0, std::string par1, std::string par2, std::string stackI__, std::string stackO__, int stackP__, int stackR__, int stackS__)
{
    // Ignore instructions after "void"
    if (stackS__ < 0)
    {
        return;
    }



    // Create dummy instructions containing the function name
    if (WDF.params.size() == 0)
    {
        if (funcName != "")
        {
            std::string s = "";

            switch (funcName[0])
            {
                case '*':
                    s = funcName.substr(1);
                    break;
                case '+':
                case '-':
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
                    s = s + " " + hex::StringGetParam(funcName, 2, '|') + "(";
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
                    //if (paramCount == 0) s = s + "void";
                    if (funcName[0] == '-')
                    {
                        s = s + ");";
                    }
                    else
                    {
                        s = s + ")";
                    }
                    s = hex::StringGetParam(funcName.substr(1), 0, '|') + s + hex::StringGetParam(funcName, 1, '|');

                    if ((funcName[0] == '+') && (addCommandBloop > 0))
                    {
                        std::string linkPrefix = hex::StringGetParam(funcName.substr(1), 0, '|');
                        std::string linkSuffix = hex::StringGetParam(funcName.substr(1), 1, '|');
                        if (addCommandBloop == 1)
                        {
                            linkPrefix = hex::StringFindReplaceFirst(linkPrefix, "(410", "(510");
                            linkSuffix = hex::StringFindReplaceFirst(linkSuffix, "(410", "(510");
                        }
                        if (addCommandBloop == 2)
                        {
                            linkPrefix = hex::StringFindReplaceFirst(linkPrefix, "(410", "(610");
                            linkSuffix = hex::StringFindReplaceFirst(linkSuffix, "(410", "(610");
                        }
                        s = s + "  " + linkPrefix + "/*bloop*/" + linkSuffix;
                    }
                    break;
            }

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
    Instr.get()->stackI__ = stackI__;
    Instr.get()->stackO__ = stackO__;
    Instr.get()->stackP__ = stackP__;
    Instr.get()->stackR__ = stackR__;
    Instr.get()->stackS__ = stackS__;

    switch (opCode)
    {
        //case 0x02: // block
        //case 0x04: // if
        case 0x05: // else
        //case 0x06: // try
        case 0x07: // catch
        case 0x19: // catch_all
        //case 0x03: // loop
        //case 0x0B: // end
            stackP__ = 0;
            stackR__ = 0;
            Instr.get()->stackP__ = 0;
            Instr.get()->stackR__ = 0;
            break;
    }

    std::vector<int> stackP_types;
    std::vector<int> stackR_types;
    {
        int stackCounter = stackP__;
        int stackPos = stackI__.size() - 1;
        int stackPos0 = stackPos + 1;
        while ((stackPos >= 0) && (stackCounter > 0))
        {
            if ((stackI__[stackPos] == '|') || (stackI__[stackPos] == '_'))
            {
                if ((stackPos0 - stackPos - 1) > 0)
                {
                    stackP_types.push_back(valueTypeNumber(stackI__.substr(stackPos + 1, stackPos0 - stackPos - 1)));
                    stackCounter--;
                }
                stackPos0 = stackPos;
            }
            stackPos--;
        }

        stackCounter = stackR__;
        stackPos = stackO__.size() - 1;
        stackPos0 = stackPos + 1;
        while ((stackPos >= 0) && (stackCounter > 0))
        {
            if ((stackO__[stackPos] == '|') || (stackO__[stackPos] == '_'))
            {
                if ((stackPos0 - stackPos - 1) > 0)
                {
                    stackR_types.push_back(valueTypeNumber(stackO__.substr(stackPos + 1, stackPos0 - stackPos - 1)));
                    stackCounter--;
                }
                stackPos0 = stackPos;
            }
            stackPos--;
        }
    }


    std::string codeParamStr = codeDef_[nameTbl][opCode & 255].paramAsm;

    bool stackSimplifiedP = false;
    bool stackSimplifiedR = false;

    if (decompOptStackSimplify)
    {
        //if (!decompOptFold)
        {
            stackSimplifiedP = true;
        }
        stackSimplifiedR = true;
    }


    switch (opCode)
    {
        case 0x02: // block
            //stackSimplifiedP = false;
            //stackSimplifiedR = false;
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
            //stackSimplifiedP = false;
            //stackSimplifiedR = false;
            Instr.get()->branchType = 2;
            break;
        case 0x03: // loop
            //stackSimplifiedP = false;
            //stackSimplifiedR = false;
            Instr.get()->branchType = 3;
            break;
        case 0x0B: // end
            //stackSimplifiedP = false;
            //stackSimplifiedR = false;
            Instr.get()->branchType = 4;
            break;
        case 0x10: // call
        case 0x11: // call_indirect
        case 0x12: // return_call
        case 0x13: // return_call_indirect
        case 0x14: // call_ref
        case 0x15: // return_call_ref
            {
                std::string parFuncName = hex::StringGetParam(par2, 2, '`');

                if ((opCode == 0x11) || (opCode == 0x13))
                {
                    Instr.get()->paramAdd(par0);
                    Instr.get()->paramAdd(par1);
                    Instr.get()->instrTextParamList = 2;
                }

                Instr.get()->name = hex::StringFindReplace(Instr.get()->name, "[#0#]", parFuncName);
            }
            break;
        case 0x0F: // return
            {
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

    if (stackSimplifiedP)
    {
        for (int i = 0; i < stackP__; i++)
        {
            Instr.get()->paramAdd(dataFieldDictionarySet("", stackP_types[stackP__ - 1 - i], "stack", stackS__ + i));
        }
    }
    else
    {
        for (int i = 0; i < stackP__; i++)
        {
            Instr.get()->paramAdd(dataFieldDictionarySet("", stackP_types[stackP__ - 1 - i], "tempp", i));
        }
    }


    Instr.get()->blockFold = false;
    Instr.get()->printComma = true;
    Instr.get()->returnName = "";
    Instr.get()->returnType = 0;
    Instr.get()->isFoldable = false;

    switch (opCode)
    {
        case 0x0F: // return
        case 0x12: // return_call
        case 0x13: // return_call_indirect
        case 0x15: // return_call_ref
            break;
        case 0x02: // block
        case 0x03: // loop
        case 0x06: // try
            //Instr.get()->paramAdd(par0);
            Instr.get()->printComma = false;
            Instr.get()->blockFold = true;
            break;
        case 0x04: // if
        case 0x05: // else
        case 0x07: // catch
        case 0x19: // catch_all
        case 0x0B: // end
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
            Instr.get()->isFoldable = true;
            break;
        case 0x21: // local.set
            Instr.get()->returnName = dataFieldDictionarySet("", 0, "local", atoi(par0.c_str()));
            Instr.get()->isFoldable = true;
            break;
        case 0x22: // local.tee
            Instr.get()->returnName = dataFieldDictionarySet("", 0, "local", atoi(par0.c_str()));
            Instr.get()->returnType = dataFieldDictionaryGetType("local", atoi(par0.c_str()));
            Instr.get()->paramAdd(dataFieldDictionaryGetVar("stack", stackS__ - 1));
            Instr.get()->isFoldable = true;
            break;
        case 0x23: // global.get
            Instr.get()->paramAdd(dataFieldDictionaryGetVar("global", atoi(par0.c_str())));
            Instr.get()->isFoldable = true;
            break;
        case 0x24: // global.set
            Instr.get()->returnName = dataFieldDictionarySet("", 0, "global", atoi(par0.c_str()));
            Instr.get()->isFoldable = true;
            break;
        case 0x41: // i32.const
        case 0x42: // i64.const
        case 0x43: // f32.const
        case 0x44: // f64.const
            Instr.get()->isFoldable = true;
            break;
    }


    switch (stackR__)
    {
        case 0:
            break;
        case 1:
            Instr.get()->returnType = stackR_types[0];
            if (Instr.get()->returnName == "")
            {
                if (stackSimplifiedR)
                {
                    Instr.get()->returnName = dataFieldDictionarySet("", Instr.get()->returnType, "stack", stackS__);
                }
                else
                {
                    Instr.get()->returnName = dataFieldDictionarySet("", Instr.get()->returnType, "tempr", 0);
                }
            }
            Instr.get()->returnNameItems.push_back(Instr.get()->returnName);
            break;
        default:
            Instr.get()->returnName = "";
            Instr.get()->returnNameItems.clear();
            Instr.get()->returnTypeList.clear();
            for (int i = 0; i < stackR__; i++)
            {
                if (i > 0) Instr.get()->returnName = Instr.get()->returnName + ", ";
                std::string returnVarName = "";
                if (stackSimplifiedR)
                {
                    returnVarName = dataFieldDictionarySet("", stackR_types[stackR__ - 1 - i], "stack", i + stackS__);
                }
                else
                {
                    returnVarName = dataFieldDictionarySet("", stackR_types[stackR__ - 1 - i], "tempr", i);
                }
                Instr.get()->returnName = Instr.get()->returnName + returnVarName;
                Instr.get()->returnNameItems.push_back(returnVarName);
                Instr.get()->returnTypeList.push_back(stackR_types[stackR__ - 1 - i]);
            }
            Instr.get()->returnName = "{" + Instr.get()->returnName + "}";
            Instr.get()->returnType = fieldTypeCallFunc;
            break;
    }

    if ((!decompOptStackSimplify) && (Instr.get()->returnName != "") && (codeParamStr != "16b") && (stackR__ == 1))
    {
        Instr.get()->isFoldable = true;
    }

    switch (opCode)
    {
        case 0x24: // global.set
            Instr.get()->isFoldable = false;
            break;
        case 0x05: // else
        case 0x07: // catch
        case 0x19: // catch_all
        case 0x0B: // end
            currentDepth--;
            {
                while (indentStack.size() <= (currentDepth))
                {
                    indentStack.push_back(wasmDecompilerIndentData());
                }
            }

            break;
        case 0x0C: // br
        case 0x0E: // br_table
        case 0x0F: // return
        case 0x12: // return_call
        case 0x13: // return_call_indirect
        case 0x15: // return_call_ref
            {
            }
            break;
    }
    Instr.get()->depth = currentDepth;

    for (int i = 0; i < size; i++)
    {
        Instr.get()->originalInstr.push_back(raw[addr + i]);
    }

    // Copy parameters from stack to temporary variables
    if (!stackSimplifiedP)
    {
        for (int i = 0; i < stackP__; i++)
        {
            WDF.additionalInstr(currentDepth, dataFieldDictionarySet("", stackP_types[stackP__ - 1 - i], "tempp", i), dataFieldDictionarySet("", stackP_types[stackP__ - 1 - i], "stack", stackS__ + i), instrId);
        }
    }

    // Execute instruction
    if (currentDepth > 0)
    {
        Instr.get()->stackPrint = true;
    }
    WDF.params.push_back(Instr);

    // Copy results from temporary variables to stack
    if (!stackSimplifiedR)
    {
        for (int i = 0; i < stackR__; i++)
        {
            WDF.additionalInstr(currentDepth, dataFieldDictionarySet("", stackR_types[stackR__ - 1 - i], "stack", stackS__ + i), dataFieldDictionarySet("", stackR_types[stackR__ - 1 - i], "tempr", i), instrId);
        }
    }

    switch (opCode)
    {
        case 0x02: // block
        case 0x03: // loop
        case 0x04: // if
        case 0x06: // try
            while (indentStack.size() <= (currentDepth))
            {
                indentStack.push_back(wasmDecompilerIndentData());
            }
            currentDepth++;
            break;
        case 0x05: // else
        case 0x07: // catch
        case 0x19: // catch_all
            while (indentStack.size() <= (currentDepth))
            {
                indentStack.push_back(wasmDecompilerIndentData());
            }
            indentStack[currentDepth].instr = opCode;
            currentDepth++;
            break;
    }
}
