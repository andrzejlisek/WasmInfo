#include "filestructure.h"

bool fileStructure::isCodeGood(sectionSubInfo &sectionSubInfo__)
{
    if (sectionSubInfo__._CodeInstr.size() > 0)
    {
        if (sectionSubInfo__._CodeInstr[sectionSubInfo__._CodeInstr.size() - 1].Opcode < 0)
        {
            return false;
        }
    }
    return true;
}

int fileStructure::parseInstruction(int ptr, sectionSubInfo &sectionSubInfo__)
{
    int InstrSize = 0;

    codeInstr codeInstr_;

    codeInstr_.Opcode = raw[ptr];
    codeInstr_.Addr = ptr;

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
    switch (codeInstr_.Opcode >> 8)
    {
        case 0x00: instrParamType = wasmDecompiler_.codeDef_[0][codeInstr_.Opcode & 255].paramAsm; break;
        case 0xFB: instrParamType = wasmDecompiler_.codeDef_[1][codeInstr_.Opcode & 255].paramAsm; break;
        case 0xFC: instrParamType = wasmDecompiler_.codeDef_[2][codeInstr_.Opcode & 255].paramAsm; break;
        case 0xFD: instrParamType = wasmDecompiler_.codeDef_[3][codeInstr_.Opcode & 255].paramAsm; break;
        case 0xFE: instrParamType = wasmDecompiler_.codeDef_[4][codeInstr_.Opcode & 255].paramAsm; break;
        case 0xFF: instrParamType = wasmDecompiler_.codeDef_[5][codeInstr_.Opcode & 255].paramAsm; break;
    }


    if (instrParamType == "")
    {
        InstrSize = 1;
    }

    if (instrParamType == "t")
    {
        instrParamType = "";
        codeInstr_.Param0 = wasmDecompiler_.valueTypeName(raw[ptr + 1]);
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
                codeInstr_.Param0 = codeInstr_.Param0 + wasmDecompiler_.valueTypeName(raw[ptr + InstrSize]);
                InstrSize++;
            }
            codeInstr_.Param0 = codeInstr_.Param0;
        }
    }


    if (instrParamType == "i32")
    {
        instrParamType = "";
        InstrSize = 1;
        codeInstr_.Param0 = std::to_string(leb128s(ptr + InstrSize, 32));
        InstrSize += leb128Size;
    }

    if (instrParamType == "i64")
    {
        instrParamType = "";
        InstrSize = 1;
        codeInstr_.Param0 = std::to_string(leb128s(ptr + InstrSize, 64));
        InstrSize += leb128Size;
    }

    if (instrParamType == "f32")
    {
        instrParamType = "";
        codeInstr_.Param0 = f32tostr(ptr + 1);
        InstrSize = 5;
    }

    if (instrParamType == "f64")
    {
        instrParamType = "";
        codeInstr_.Param0 = f64tostr(ptr + 1);
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
            codeInstr_.Param0 = "{";
            for (int i = 0; i < vecSize; i++)
            {
                if (i > 0)
                {
                    codeInstr_.Param0 = codeInstr_.Param0 + ",";
                }
                codeInstr_.Param0 = codeInstr_.Param0 + std::to_string(leb128u(ptr + InstrSize));
                InstrSize += leb128Size;
            }
            codeInstr_.Param0 = codeInstr_.Param0 + "}";
        }
        codeInstr_.Param1 = std::to_string(leb128u(ptr + InstrSize));
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
    sectionSubInfo__._CodeInstr.push_back(codeInstr_);
    return codeInstr_.Size;
}

int fileStructure::parseInstructions(int addr, sectionSubInfo &sectionSubInfo__)
{
    int BlockDepth = 1;
    int testN = 1000000000;
    int ptr = addr;
    int codeLength = 0;
    while ((BlockDepth > 0) && (testN > 0))
    {
        int InstrS = parseInstruction(ptr, sectionSubInfo__);
        switch (sectionSubInfo__._CodeInstr[sectionSubInfo__._CodeInstr.size() - 1].Opcode)
        {
            case 0x05: BlockDepth--; break;
            case 0x0B: BlockDepth--; break;
            case -1: testN = 0; break;
        }
        sectionSubInfo__._CodeInstr[sectionSubInfo__._CodeInstr.size() - 1].Depth = BlockDepth;
        switch (sectionSubInfo__._CodeInstr[sectionSubInfo__._CodeInstr.size() - 1].Opcode)
        {
            case 0x02: BlockDepth++; break;
            case 0x03: BlockDepth++; break;
            case 0x04: BlockDepth++; break;
            case 0x05: BlockDepth++; break;
        }

        ptr += InstrS;
        codeLength += InstrS;


        testN--;
    }
    return codeLength;
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

    text = hex::StringFindReplace(text, "[~0~]", codeInstr_.Param0);
    text = hex::StringFindReplace(text, "[~1~]", codeInstr_.Param1);
    text = hex::StringFindReplace(text, "[~2~]", codeInstr_.Param2);
    if (hex::StringIndexOf(text, "[#0#]") > 0)
    {
        text = hex::StringFindReplace(text, "[#0#]", getFunctionNameById(-1, atoi(codeInstr_.Param0.c_str()), 0));
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
        wasmDecompiler_.codeDef_[setType][i].nameAsm = "";
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
}
