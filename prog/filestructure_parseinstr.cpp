#include "filestructure.h"

int fileStructure::parseInstruction(int ptr, sectionSubInfo &sectionSubInfo__)
{
    int InstrSize = 0;

    codeInstr codeInstr_;

    codeInstr_.Opcode = raw[ptr];
    codeInstr_.Addr = ptr;
    codeInstr_.errorMsg = "";

    std::string instrParamType = "";
    bool additionalSIMD = false;
    switch (codeInstr_.Opcode)
    {
        case 0xFB:
        case 0xFC:
        case 0xFD:
        case 0xFE:
            ptr++;
            codeInstr_.Opcode = codeInstr_.Opcode << 8;
            codeInstr_.Opcode = codeInstr_.Opcode + ((int)raw[ptr]);


            // Distinguish the SIMD planes
            if ((raw[ptr - 1] == 0xFD) && (raw[ptr] >= 0x80))
            {
                additionalSIMD = true;
                if (raw[ptr + 1] == 0x02)
                {
                    codeInstr_.Opcode = codeInstr_.Opcode + 0x0200;
                }
            }


            break;
    }
    wasmDecompiler::codeDef codeDef__;
    switch (codeInstr_.Opcode >> 8)
    {
        case 0x00: codeDef__ = wasmDecompiler_.codeDef_[0][codeInstr_.Opcode & 255]; break;
        case 0xFB: codeDef__ = wasmDecompiler_.codeDef_[1][codeInstr_.Opcode & 255]; break;
        case 0xFC: codeDef__ = wasmDecompiler_.codeDef_[2][codeInstr_.Opcode & 255]; break;
        case 0xFD: codeDef__ = wasmDecompiler_.codeDef_[3][codeInstr_.Opcode & 255]; break;
        case 0xFE: codeDef__ = wasmDecompiler_.codeDef_[4][codeInstr_.Opcode & 255]; break;
        case 0xFF: codeDef__ = wasmDecompiler_.codeDef_[5][codeInstr_.Opcode & 255]; break;
    }
    instrParamType = codeDef__.paramAsm;


    if (instrParamType == "")
    {
        InstrSize = 1;
    }

    if (instrParamType == "t")
    {
        instrParamType = "";
        codeInstr_.Param0 = valueTypeNameEx(raw[ptr + 1]);
        codeInstr_.Param1 = valueTypeNameEx(0 - (int)(raw[ptr + 1]));
        InstrSize = 2;
    }

    if (instrParamType == "tx")
    {
        instrParamType = "";
        InstrSize = 1;
        {
            int vecSize = leb128u(ptr + InstrSize);
            InstrSize += leb128Size;
            codeInstr_.Param0 = "";
            for (int i = 0; i < vecSize; i++)
            {
                if (i > 0)
                {
                    codeInstr_.Param0 = codeInstr_.Param0 + ",";
                }
                codeInstr_.Param0 = codeInstr_.Param0 + valueTypeNameEx(raw[ptr + InstrSize]);
                InstrSize++;
            }
            codeInstr_.Param0 = codeInstr_.Param0;
        }
    }


    if (instrParamType == "i32")
    {
        instrParamType = "";
        InstrSize = 1;
        codeInstr_.Param0 = wasmDecompiler_.dataFieldDictionarySet(std::to_string(leb128s(ptr + InstrSize, 32)), wasmDecompiler_.fieldType_i32, "const", 0);
        InstrSize += leb128Size;
    }

    if (instrParamType == "i64")
    {
        instrParamType = "";
        InstrSize = 1;
        codeInstr_.Param0 = wasmDecompiler_.dataFieldDictionarySet(std::to_string(leb128s(ptr + InstrSize, 64)), wasmDecompiler_.fieldType_i64, "const", 0);
        InstrSize += leb128Size;
    }

    if (instrParamType == "f32")
    {
        instrParamType = "";
        codeInstr_.Param0 = wasmDecompiler_.dataFieldDictionarySet(f32tostr(ptr + 1), wasmDecompiler_.fieldType_f32, "const", 0);
        InstrSize = 5;
    }

    if (instrParamType == "f64")
    {
        instrParamType = "";
        codeInstr_.Param0 = wasmDecompiler_.dataFieldDictionarySet(f64tostr(ptr + 1), wasmDecompiler_.fieldType_f64, "const", 0);
        InstrSize = 9;
    }

    if (instrParamType == "u")
    {
        instrParamType = "";
        InstrSize = 1;
        codeInstr_.Param0 = std::to_string(leb128u(ptr + 1));
        InstrSize += leb128Size;
    }

    if (instrParamType == "uxu")
    {
        instrParamType = "";
        InstrSize = 1;
        {
            int vecSize = leb128u(ptr + InstrSize);
            InstrSize += leb128Size;
            codeInstr_.Param0 = "";
            for (int i = 0; i < vecSize; i++)
            {
                codeInstr_.Param0 = codeInstr_.Param0 + std::to_string(leb128u(ptr + InstrSize));
                codeInstr_.Param0 = codeInstr_.Param0 + ", ";
                InstrSize += leb128Size;
            }
        }
        codeInstr_.Param0 = codeInstr_.Param0 + std::to_string(leb128u(ptr + InstrSize));
        InstrSize += leb128Size;
    }

    if (instrParamType == "uu")
    {
        instrParamType = "";
        InstrSize = 1;
        codeInstr_.Param0 = std::to_string(leb128u(ptr + InstrSize));
        InstrSize += leb128Size;
        codeInstr_.Param1 = std::to_string(leb128u(ptr + InstrSize));
        InstrSize += leb128Size;
    }

    if (instrParamType == "uub")
    {
        instrParamType = "";
        InstrSize = 1;
        codeInstr_.Param0 = std::to_string(leb128u(ptr + InstrSize));
        InstrSize += leb128Size;
        codeInstr_.Param1 = std::to_string(leb128u(ptr + InstrSize));
        InstrSize += leb128Size;
        codeInstr_.Param2 = std::to_string((int)(raw[ptr + InstrSize]));
        InstrSize += 1;
    }

    if (instrParamType == "b")
    {
        instrParamType = "";
        InstrSize = 1;
        codeInstr_.Param0 = std::to_string((int)(raw[ptr + InstrSize]));
        InstrSize += 1;
    }

    if (instrParamType == "16b")
    {
        instrParamType = "";
        InstrSize = 1;
        codeInstr_.Param0 = "";
        for (int i = 0; i < 16; i++)
        {
            if (i > 0)
            {
                codeInstr_.Param0 = codeInstr_.Param0 + ", ";
            }
            codeInstr_.Param0 = codeInstr_.Param0 + std::to_string((int)(raw[ptr + InstrSize]));
            InstrSize += 1;
        }
    }

    if (instrParamType != "")
    {
        InstrSize = 0;
    }

    codeInstr_.Size = InstrSize;
    if (InstrSize == 0)
    {
        codeInstr_.Size = 8;
        codeInstr_.Opcode = -1;
    }
    else
    {
        if (codeInstr_.Opcode > 255)
        {
            codeInstr_.Size++;
        }
        if (additionalSIMD)
        {
            codeInstr_.Size++;
        }
    }


    codeInstr_.stackP = atoi(codeDef__.stackParam.c_str());
    codeInstr_.stackR = atoi(codeDef__.stackResult.c_str());
    codeInstr_.stackR_ + "???";
    if (codeInstr_.stackR == 0)
    {
        codeInstr_.stackR_ = "";
    }
    if (codeInstr_.stackR == 1)
    {
        codeInstr_.stackR_ = wasmDecompiler_.valueTypeName(codeDef__.stackResultType);
        if (codeDef__.stackResultType < 0) codeInstr_.stackR_ = wasmDecompiler_.valueTypeName(0);
    }


    codeInstr_.stackI_ = wasmDecompiler_.currentStackPrint();
    codeInstr_.stackS = wasmDecompiler_.currentStackSize(0) - codeInstr_.stackP;

    // Analyze stack behavior depending on special instructions
    switch (codeInstr_.Opcode)
    {
        case 0x04: // if
            {
                if (!wasmDecompiler_.currentStackPop())
                {
                    codeInstr_.errorMsg = codeInstr_.errorMsg + " !!! STACK POP ERROR !!!";
                }
                if (!wasmDecompiler_.currentStackBlockPush(atoi(hex::StringGetParam(codeInstr_.Param1, 0, '|').c_str()), atoi(hex::StringGetParam(codeInstr_.Param1 + "|", 1, '|').c_str()), hex::StringGetParam(codeInstr_.Param0, 0, '|'), hex::StringGetParam(codeInstr_.Param0 + "|", 1, '|')))
                {
                    codeInstr_.errorMsg = codeInstr_.errorMsg + " !!! STACK ERROR !!!";
                }
            }
            break;
        case 0x02: // block
        case 0x03: // loop
        case 0x06: // try
            {
                if (!wasmDecompiler_.currentStackBlockPush(atoi(hex::StringGetParam(codeInstr_.Param1, 0, '|').c_str()), atoi(hex::StringGetParam(codeInstr_.Param1 + "|", 1, '|').c_str()), hex::StringGetParam(codeInstr_.Param0, 0, '|'), hex::StringGetParam(codeInstr_.Param0 + "|", 1, '|')))
                {
                    codeInstr_.errorMsg = codeInstr_.errorMsg + " !!! STACK ERROR !!!";
                }
            }
            break;
        case 0x05: // else
            {
                // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                codeInstr_.stackS = wasmDecompiler_.currentStackSize(0) - wasmDecompiler_.currentStackSize(1);
                codeInstr_.stackP = wasmDecompiler_.currentStackSize(1);
                wasmDecompiler_.currentStackBlockRestore(false);
                codeInstr_.stackR = wasmDecompiler_.currentStackSize(1);
            }
            break;
        case 0x19: // catch_all
            {
                // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                codeInstr_.stackS = wasmDecompiler_.currentStackSize(0) - wasmDecompiler_.currentStackSize(1);
                codeInstr_.stackP = wasmDecompiler_.currentStackSize(1);
                wasmDecompiler_.currentStackBlockRestore(true);
                codeInstr_.stackR = wasmDecompiler_.currentStackSize(1);
            }
            break;
        case 0x07: // catch
            {
                // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                codeInstr_.stackS = wasmDecompiler_.currentStackSize(0) - wasmDecompiler_.currentStackSize(1);
                codeInstr_.stackP = wasmDecompiler_.currentStackSize(1);
                wasmDecompiler_.currentStackBlockRestore(true);
                codeInstr_.stackR = wasmDecompiler_.currentStackSize(1);

                int catchTag = atoi(codeInstr_.Param0.c_str());
                int catchTypeIdx = getTypeListItemByTag(catchTag);
                if (catchTypeIdx >= 0)
                {
                    if (sectionSubInfo_[catchTypeIdx]._TypeParams.size() > 0)
                    {
                        for (int iiii = 0; iiii < sectionSubInfo_[catchTypeIdx]._TypeParams.size(); iiii++)
                        {
                            wasmDecompiler_.currentStackPush(sectionSubInfo_[catchTypeIdx]._TypeParams[iiii]);
                            codeInstr_.stackR++;
                        }
                    }
                }

            }
            break;
        case 0x0B: // end
            {
                if (!wasmDecompiler_.currentStackBlockPop())
                {
                    codeInstr_.errorMsg = codeInstr_.errorMsg + " !!! STACK ERROR !!!";
                }
            }
            break;



        case 0x10: // call
        case 0x11: // call_indirect
        case 0x12: // return_call
        case 0x13: // return_call_indirect
        case 0x14: // call_ref
        case 0x15: // return_call_ref
            {
                int typeIdx = 0;
                int dummy = -3;
                int stackP1 = 0;
                switch (codeInstr_.Opcode)
                {
                    case 0x11: // call_indirect
                    case 0x13: // return_call_indirect
                        stackP1 = 1;
                        typeIdx = atoi(getFunctionType(atoi(codeInstr_.Param0.c_str()), "{~}", dummy).c_str());
                        break;
                    default:
                        typeIdx = atoi(getFunctionNameById(-1, atoi(codeInstr_.Param0.c_str()), -3).c_str());
                        break;
                }

                codeInstr_.stackP = sectionSubInfo_[typeIdx]._TypeParams.size() + stackP1;
                codeInstr_.stackR = sectionSubInfo_[typeIdx]._TypeReturn.size();
                codeInstr_.stackS = codeInstr_.stackS - codeInstr_.stackP;

                for (int i = 0; i < codeInstr_.stackP; i++)
                {
                    if (!wasmDecompiler_.currentStackPop())
                    {
                        codeInstr_.errorMsg = codeInstr_.errorMsg + " !!! STACK POP ERROR !!!";
                    }
                }

                for (int i = 0; i < codeInstr_.stackR; i++)
                {
                    wasmDecompiler_.currentStackPush(sectionSubInfo_[typeIdx]._TypeReturn[i]);
                }
            }

            break;
        case 0x20: // local.get
            codeInstr_.stackR_ = wasmDecompiler_.valueTypeName(wasmDecompiler_.dataFieldDictionaryGetType("local", atoi(codeInstr_.Param0.c_str())) & 255);
            wasmDecompiler_.currentStackPush(wasmDecompiler_.valueTypeNumber(codeInstr_.stackR_));
            break;

        case 0x23: // global.get
            codeInstr_.stackR_ = wasmDecompiler_.valueTypeName(wasmDecompiler_.dataFieldDictionaryGetType("global", atoi(codeInstr_.Param0.c_str())) & 255);
            wasmDecompiler_.currentStackPush(wasmDecompiler_.valueTypeNumber(codeInstr_.stackR_));
            break;

        case 0x1B: // select
            if (!wasmDecompiler_.currentStackPop())
            {
                codeInstr_.errorMsg = codeInstr_.errorMsg + " !!! STACK POP ERROR !!!";
            }
            if (!wasmDecompiler_.currentStackPop())
            {
                codeInstr_.errorMsg = codeInstr_.errorMsg + " !!! STACK POP ERROR !!!";
            }
            break;

        case 0x1C: // select_type
            if (!wasmDecompiler_.currentStackPop())
            {
                codeInstr_.errorMsg = codeInstr_.errorMsg + " !!! STACK POP ERROR !!!";
            }
            if (!wasmDecompiler_.currentStackPop())
            {
                codeInstr_.errorMsg = codeInstr_.errorMsg + " !!! STACK POP ERROR !!!";
            }
            break;

        case 0xD0: // ref.null
            wasmDecompiler_.currentStackPush(raw[ptr + 1]);
            break;

        default:
            for (int i = 0; i < codeInstr_.stackP; i++)
            {
                if (!wasmDecompiler_.currentStackPop())
                {
                    codeInstr_.errorMsg = codeInstr_.errorMsg + " !!! STACK POP ERROR !!!";
                }
            }
            if (codeInstr_.stackR == 1)
            {
                wasmDecompiler_.currentStackPush(wasmDecompiler_.valueTypeNumber(codeInstr_.stackR_));
            }
            break;
    }

    // Ignore stack behavior after the instruction
    switch (codeInstr_.Opcode)
    {
        case 0x00: // unreachable
        case 0x08: // throw
        case 0x09: // rethrow
        case 0x0C: // br
        case 0x0F: // return
        case 0x12: // return_call
        case 0x13: // return_call_indirect
            wasmDecompiler_.currentStackIgnore();
            break;
    }

    codeInstr_.stackO_ = wasmDecompiler_.currentStackPrint();
    if (codeInstr_.stackS != (wasmDecompiler_.currentStackSize(0) - codeInstr_.stackR))
    {
        if ((wasmDecompiler_.currentStackSize(0) >= 0) && (codeInstr_.stackS >= 0))
        {
            codeInstr_.errorMsg = codeInstr_.errorMsg + " !!! STACK SIZE ERROR !!!";
            codeInstr_.errorMsg = codeInstr_.errorMsg + " stackSize0=" + std::to_string(wasmDecompiler_.currentStackSize(0)) + " ";
            codeInstr_.errorMsg = codeInstr_.errorMsg + " stackSize1=" + std::to_string(wasmDecompiler_.currentStackSize(1)) + " ";
            codeInstr_.errorMsg = codeInstr_.errorMsg + " stackP=" + std::to_string(codeInstr_.stackP) + " ";
            codeInstr_.errorMsg = codeInstr_.errorMsg + " stackS=" + std::to_string(codeInstr_.stackS) + " ";
            codeInstr_.errorMsg = codeInstr_.errorMsg + " stackR=" + std::to_string(codeInstr_.stackR) + " ";
        }
    }
    if ((codeInstr_.stackS < 0))
    {
        codeInstr_.stackS = wasmDecompiler_.currentStackSize(0);
    }

    if (wasmDecompiler_.codeInstrInfoStack)
    {
        wasmDecompiler_.stackPrintInfoFull(codeInstr_.stackS, codeInstr_.stackI_, codeInstr_.stackP, codeInstr_.stackO_, codeInstr_.stackR);
    }


    sectionSubInfo__._CodeInstr.push_back(codeInstr_);
    return codeInstr_.Size;
}

int fileStructure::parseInstructions(int addr, sectionSubInfo &sectionSubInfo__, int sectionSubInfoI, int returnTypeX)
{
    int BlockDepth = 1;
    int testN = 1000000000;
    int ptr = addr;
    int codeLength = 0;
    wasmDecompiler_.reset("", 0, sectionSubInfo__._CodeDataFieldDictionary);
    wasmDecompiler_.dataFieldDictionary.clear();

    int localNum = 0;
    getFunctionName(sectionSubInfo__.Index, 0, localNum);
    {
        int iii = 0;
        while (getVarTypeG(iii) != 0)
        {
            wasmDecompiler_.dataFieldDictionarySet(getGlobalVarName(iii, false, 0), getVarTypeG(iii), "global", iii);
            iii++;
        }
    }
    for (int iii = 0; iii < localNum; iii++)
    {
        wasmDecompiler_.dataFieldDictionarySet("", getVarTypeL(iii, sectionSubInfoI, sectionSubInfo__), "param", iii);
    }
    for (int iii = 0; iii < sectionSubInfo__._CodeLocalN.size(); iii++)
    {
        for (int iiii = 0; iiii < sectionSubInfo__._CodeLocalN[iii]; iiii++)
        {
            wasmDecompiler_.dataFieldDictionarySet("", getVarTypeL(localNum, sectionSubInfoI, sectionSubInfo__), "local", localNum);
            localNum++;
        }
    }

    std::string returnStr = "";
    int returnNum = 0;
    if (returnTypeX == 1)
    {
        while (getVarTypeL(returnNum, -1, sectionSubInfo__) > 0)
        {
            if (returnNum > 0) returnStr = returnStr + "_";
            returnStr = returnStr + wasmDecompiler_.valueTypeName(getVarTypeL(returnNum, -1, sectionSubInfo__));
            returnNum++;
        }
    }
    if (returnTypeX == 2)
    {
        returnNum = 1;
        returnStr = wasmDecompiler_.valueTypeName(sectionSubInfo__._TypeReturn[0]);
    }
    if (returnTypeX == 3)
    {
        returnNum = 0;
        returnStr = "void";
    }
    wasmDecompiler_.currentStackBlockPrepare(0, returnNum, "", returnStr + "_");
    if (returnTypeX == 3)
    {
        returnStr = "";
    }


    while ((BlockDepth > 0) && (testN > 0))
    {
        int InstrS = parseInstruction(ptr, sectionSubInfo__);
        switch (sectionSubInfo__._CodeInstr[sectionSubInfo__._CodeInstr.size() - 1].Opcode)
        {
            case 0x05: BlockDepth--; break; // else
            case 0x07: BlockDepth--; break; // catch
            case 0x19: BlockDepth--; break; // catch_all

            case 0x0B: BlockDepth--; break; // end

            case -1: testN = 0; break;
        }
        sectionSubInfo__._CodeInstr[sectionSubInfo__._CodeInstr.size() - 1].Depth = BlockDepth;
        switch (sectionSubInfo__._CodeInstr[sectionSubInfo__._CodeInstr.size() - 1].Opcode)
        {
            case 0x02: BlockDepth++; break; // block
            case 0x03: BlockDepth++; break; // loop
            case 0x04: BlockDepth++; break; // if
            case 0x06: BlockDepth++; break; // try

            case 0x05: BlockDepth++; break; // else
            case 0x07: BlockDepth++; break; // catch
            case 0x19: BlockDepth++; break; // catch_all
        }

        ptr += InstrS;
        codeLength += InstrS;


        testN--;
    }


    // Check stack result
    if (sectionSubInfo__._CodeInstr[sectionSubInfo__._CodeInstr.size() - 1].stackO_ != returnStr)
    {
        sectionSubInfo__._CodeInstr[sectionSubInfo__._CodeInstr.size() - 1].errorMsg += " !!! END STACK ERROR !!!";
    }


    sectionSubInfo__._CodeDataFieldDictionary.clear();
    for (int i = 0; i < wasmDecompiler_.dataFieldDictionary.size(); i++)
    {
        sectionSubInfo__._CodeDataFieldDictionary.push_back(wasmDecompiler_.dataFieldDictionary[i]);
    }
    return codeLength;
}
