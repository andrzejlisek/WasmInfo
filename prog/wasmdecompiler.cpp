#include "wasmdecompiler.h"

wasmDecompiler::wasmDecompiler()
{

}

std::string wasmDecompiler::valueTypeName(int typeSig)
{
    if (typeSig >= 256)
    {
        return "const " + valueTypeName(typeSig - 256);
    }
    switch (typeSig)
    {
        case 0x00: return "value";
        case 0x40: return "void";
        case 0x7F: return "i32";
        case 0x7E: return "i64";
        case 0x7D: return "f32";
        case 0x7C: return "f64";
        case 0x7B: return "v128";
        case 0x70: return "funcref";
        case 0x6F: return "externref";
        case 0x69: return "exnref";
        default:   return "0x" + hex::IntToHex8(typeSig);
    }
}

void wasmDecompiler::reset(std::string funcName_)
{
    lastOpcode = 0;
    funcName = funcName_;
    dataGlobal.clear();
    dataLocal.clear();
    dataParam.clear();
    dataReturn.clear();

    WDF.depth = 0;
    WDF.isFoldable = false;
    WDF.name = "root";
    WDF.returnName = "root";
    WDF.params.clear();

    currentDepth = 1;
    currentStack = 0;
    stackSizeBlock.clear();
    tempVarCounter = 1;
}

void wasmDecompiler::addCommand(unsigned char * raw, int addr, int size, std::string par0, std::string par1, std::string par2)
{
    // Adding final "end" command requires adding additional "return command" when the function returns value and the stack is not empty
    if ((dataReturn.size() > 0) && (currentDepth == 1) && (raw[addr] == 0x0B) && (currentStack > 0))
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


    // Create dummy instructions containing the function name
    if (WDF.params.size() == 0)
    {
        if (funcName != "")
        {
            std::string s = "";
            if (dataReturn.size() == 0) s = "void";
            for (int i = 0; i < dataReturn.size(); i++)
            {
                if (i > 0)
                {
                    s = s + "_";
                }
                s = s + valueTypeName(dataReturn[i].type);
            }
            s = s + " " + funcName + "(";
            if (dataParam.size() == 0) s = s + "void";
            for (int i = 0; i < dataParam.size(); i++)
            {
                if (i > 0)
                {
                    s = s + ", ";
                }
                s = s + valueTypeName(dataParam[i].type) + " " + dataParam[i].name;
            }
            s = s + ")";
            WDF.additionalInstr(0, "", s, 0, "");
        }
        WDF.additionalInstr(0, "", "{", 0, "");
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

    bool stackSimplified = useSimplified;

    switch (opCode)
    {
        case 0x02: // block
        case 0x03: // loop
        case 0x04: // if
        case 0x05: // else
        case 0x0B: // end
            stackSimplified = false;
            break;
        case 0x10: // call
        case 0x11: // call_indirect
        case 0x12: // return_call
        case 0x13: // return_call_indirect
        case 0x14: // call_ref
        case 0x15: // return_call_ref
            {
                int idxFind1 = par2.find("`");
                int idxFind2 = par2.find("`", idxFind1 + 1);

                std::string parStackP = par2.substr(0, idxFind1);
                std::string parStackR = par2.substr(idxFind1 + 1, idxFind2 - idxFind1 - 1);
                std::string parFuncName = par2.substr(idxFind2 + 1);

                stackP = atoi(parStackP.c_str());
                stackR = atoi(parStackR.c_str());

                if ((opCode == 0x11) || (opCode == 0x13))
                {
                    stackP++;
                }

                Instr.get()->name = hex::StringFindReplace(Instr.get()->name, "[#0#]", parFuncName);
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
                if ((codeParamStr == "u") || (codeParamStr == "b") || (codeParamStr == "16b"))
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
            Instr.get()->paramAdd("stack" + std::to_string(stackP - 1 - i));
        }
    }
    else
    {
        for (int i = 0; i < stackP; i++)
        {
            Instr.get()->paramAdd("param" + std::to_string(i));
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
            if (Instr.get()->params.size() > dataReturn.size())
            {
                int n = Instr.get()->params.size() - dataReturn.size();
                Instr.get()->params.erase(Instr.get()->params.begin(), Instr.get()->params.begin() + n);
            }
            break;
        case 0x02: // block
        case 0x03: // loop
            Instr.get()->paramAdd(par0);
            Instr.get()->printComma = false;
            Instr.get()->blockFold = true;
            break;
        case 0x04: // if
        case 0x05: // else
        case 0x0B: // end
            Instr.get()->printComma = false;
            Instr.get()->blockFold = true;
            break;
        case 0x0C: // br
        case 0x0D: // br_if
            Instr.get()->paramAdd(par0);
            break;
        case 0x20: // local.get
            Instr.get()->paramAdd("local" + par0);
            break;
        case 0x21: // local.set
            Instr.get()->returnName = "local" + par0;
            break;
        case 0x22: // local.tee
            Instr.get()->returnName = "local" + par0;
            Instr.get()->paramAdd("stack0");
            break;
        case 0x23: // global.get
            Instr.get()->paramAdd("global" + par0);
            break;
        case 0x24: // global.set
            Instr.get()->returnName = "global" + par0;
            break;
    }

    Instr.get()->isFoldable = false;
    if ((stackR == 1) && (Instr.get()->returnName == ""))
    {
        Instr.get()->returnName = resultVarPrefix + std::to_string(tempVarCounter);
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
            Instr.get()->returnName = Instr.get()->returnName + resultVarPrefix + std::to_string(i + tempVarCounter);
            Instr.get()->returnNameItems.push_back(resultVarPrefix + std::to_string(i + tempVarCounter));
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
        case 0x05: currentDepth--; break;
        case 0x0B: currentDepth--; break;
    }
    Instr.get()->depth = currentDepth;

    for (int i = 0; i < size; i++)
    {
        Instr.get()->originalInstr.push_back(raw[addr + i]);
    }



    if (!stackSimplified)
    {
        // Copy parameters from stack to temporary variables
        for (int i = 0; i < stackP; i++)
        {
            WDF.additionalInstr(currentDepth, "param" + std::to_string(i), "stack" + std::to_string(stackP - 1 - i), instrId, "");
        }

        // Remove parameters from the stack
        if (stackP > 0)
        {
            currentStack -= stackP;
            for (int i = 0; i < currentStack; i++)
            {
                WDF.additionalInstr(currentDepth, "stack" + std::to_string(i), "stack" + std::to_string(i + stackP), instrId, "");
            }
        }
    }

    // Handle "if" and "else" stack size
    switch (opCode)
    {
        case 0x04: // if
            while (stackSizeBlock.size() <= currentDepth)
            {
                stackSizeBlock.push_back(0);
            }
            stackSizeBlock[currentDepth] = currentStack;
            break;
        case 0x05: // else
            while (stackSizeBlock.size() <= currentDepth)
            {
                stackSizeBlock.push_back(0);
            }
            currentStack = stackSizeBlock[currentDepth];
            break;
    }

    // Execute instruction
    WDF.params.push_back(Instr);

    if (stackSimplified)
    {
        // More parameters are more than results ==> Remove stack items
        if (stackP > stackR)
        {
            currentStack -= (stackP - stackR);
            for (int i = stackR; i < currentStack; i++)
            {
                WDF.additionalInstr(currentDepth, "stack" + std::to_string(i), "stack" + std::to_string(i + stackP - stackR), instrId, "");
            }
        }

        // Less parameters are more than results ==> Add stack items
        if (stackP < stackR)
        {
            for (int i = currentStack - 1; i >= stackP; i--)
            {
                WDF.additionalInstr(currentDepth, "stack" + std::to_string(i + stackR - stackP), "stack" + std::to_string(i), instrId, "");
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
                WDF.additionalInstr(currentDepth, "stack" + std::to_string(i + stackR), "stack" + std::to_string(i), instrId, "");
            }
            currentStack += stackR;
        }
    }

    // Copy result from temporary variables to stack
    for (int i = 0; i < stackR; i++)
    {
        WDF.additionalInstr(currentDepth, "stack" + std::to_string(stackR - 1 - i), resultVarPrefix + std::to_string(i + tempVarCounter), instrId, "");
    }

    switch (opCode)
    {
        case 0x02: currentDepth++; break;
        case 0x03: currentDepth++; break;
        case 0x04: currentDepth++; break;
        case 0x05: currentDepth++; break;
    }

    tempVarCounter = tempVarCounter + stackR;
}

void wasmDecompiler::addGlobal(int idx, int dataType)
{
    dataField dataField_;
    dataField_.name = "global" + std::to_string(idx);
    dataField_.type = dataType;
    dataGlobal.push_back(dataField_);
}

void wasmDecompiler::addParam(int idx, int dataType)
{
    dataField dataField_;
    dataField_.name = "local" + std::to_string(idx);
    dataField_.type = dataType;
    dataParam.push_back(dataField_);
}

void wasmDecompiler::addLocal(int idx, int dataType)
{
    dataField dataField_;
    dataField_.name = "local" + std::to_string(idx);
    dataField_.type = dataType;
    dataLocal.push_back(dataField_);
}

void wasmDecompiler::addReturn(int idx, int dataType)
{
    dataField dataField_;
    dataField_.name = "return" + std::to_string(idx);
    dataField_.type = dataType;
    dataReturn.push_back(dataField_);
}

void wasmDecompiler::codeOptimize()
{
    int work = debugOptimizeIterations;
    while (work > 0)
    {
        work--;

        // Find first foldable function
        int foldableFunc = -1;
        for (int i = 0; i < WDF.params.size(); i++)
        {
            if (WDF.params[i].get()->isFoldable)
            {
                foldableFunc = i;
                break;
            }
        }

        // Find the first unfoldable function or second result assignment
        // Find the function to fold
        int spaceBorder = WDF.params.size() - 1;
        int assign = -1;
        int assignPar = -1;
        std::string varFold = "~";
        if (foldableFunc >= 0)
        {
            varFold = WDF.params[foldableFunc].get()->returnName;
            for (int i = (foldableFunc + 1); i < WDF.params.size(); i++)
            {
                for (int ii = 0; ii < WDF.params[i].get()->params.size(); ii++)
                {
                    if (WDF.params[i].get()->params[ii].get()->name == varFold)
                    {
                        if (assign < 0)
                        {
                            assign = i;
                            assignPar = ii;
                        }
                        else
                        {
                            assign = WDF.params.size() + 1;
                        }
                    }
                }
                if (WDF.params[i].get()->blockFold)
                {
                    spaceBorder = i;
                    break;
                }
                bool loopBreak = false;
                for (int ii = 0; ii < WDF.params[i].get()->returnNameItems.size(); ii++)
                {
                    if (WDF.params[i].get()->returnNameItems[ii] == varFold)
                    {
                        spaceBorder = i;
                        loopBreak = true;
                    }
                }
                if (loopBreak)
                {
                    break;
                }
            }
        }


        // Fold function
        work = 0 - work;
        if ((spaceBorder >= assign) && (assign > foldableFunc) && (foldableFunc >= 0))
        {
            WDF.params[assign].get()->params[assignPar] = WDF.params[foldableFunc];
            WDF.params.erase(WDF.params.begin() + foldableFunc);
            work = 0 - work;
        }
        else
        {
            if (foldableFunc >= 0)
            {
                WDF.params[foldableFunc].get()->isFoldable = false;
                work = 0 - work;
            }
        }
    }


    // Write the local variable types
    for (int i = 0; i < WDF.params.size(); i++)
    {
        if (WDF.params[i].get()->returnName.size() > 5)
        {
            if (WDF.params[i].get()->returnName.substr(0, 5) == "local")
            {
                for (int ii = 0; ii < dataLocal.size(); ii++)
                {
                    if ((WDF.params[i].get()->returnName == dataLocal[ii].name) && (dataLocal[ii].type >= 0))
                    {
                        WDF.params[i].get()->returnName = valueTypeName(dataLocal[ii].type) + " " + WDF.params[i].get()->returnName;
                        dataLocal[ii].type = 0;
                    }
                }
            }
            break;
        }
    }

}

std::string wasmDecompiler::debugValues(int idx)
{
    return "";

    std::string s;

    if (idx == (WDF.params.size() + 0))
    {
        s = "global: ";
        for (int i = 0; i < dataGlobal.size(); i++)
        {
            s = s + valueTypeName(dataGlobal[i].type) + " " + dataGlobal[i].name + "; ";
        }
        return s;
    }

    if (idx == (WDF.params.size() + 1))
    {
        s = "param: ";
        for (int i = 0; i < dataParam.size(); i++)
        {
            s = s + valueTypeName(dataParam[i].type) + " " + dataParam[i].name + "; ";
        }
        return s;
    }

    if (idx == (WDF.params.size() + 2))
    {
        s = "local: ";
        for (int i = 0; i < dataLocal.size(); i++)
        {
            s = s + valueTypeName(dataLocal[i].type) + " " + dataLocal[i].name + "; ";
        }
        return s;
    }

    if (idx == (WDF.params.size() + 3))
    {
        s = "return: ";
        for (int i = 0; i < dataReturn.size(); i++)
        {
            s = s + valueTypeName(dataReturn[i].type) + " " + dataReturn[i].name + "; ";
        }
        return s;
    }
}

std::string wasmDecompiler::printCommand(int idx)
{
    if (idx == 0)
    {
        codeOptimize();
    }

    std::string s = debugValues(idx);
    if (s != "") return s;

    if (idx < WDF.params.size())
    {
        s = hex::indent(WDF.params[idx].get()->depth);
        if (WDF.params[idx].get()->returnName != "")
        {
            s = s + WDF.params[idx].get()->returnName + " = ";
        }
        s = s + WDF.params[idx].get()->instrText();
        s = hex::StringFindReplace(s, "[\\n]", "[\\*]" + hex::indent(WDF.params[idx].get()->depth));
        s = hex::StringFindReplace(s, "[\\*]", "[\\n]");
        if (debugInfo)
        {
            std::string debugPrefix = std::to_string(WDF.params[idx].get()->id) + (WDF.params[idx].get()->isFoldable ? " " : "*") + (WDF.params[idx].get()->blockFold ? "#" : " ") + "   ";
            s = debugPrefix + hex::StringFindReplace(s, "[\\n]", "[\\*]" + debugPrefix);
            s = hex::StringFindReplace(s, "[\\*]", "[\\n]");
        }
        if (WDF.params[idx].get()->printComma)
        {
            s = s + ";";
        }
        if (WDF.params[idx].get()->comment != "")
        {
            s = s + "  // " + WDF.params[idx].get()->comment;
        }
        return s;
    }

    return "`";
}
