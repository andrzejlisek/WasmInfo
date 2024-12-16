#include "filestructure.h"

void fileStructure::parseType(sectionInfo &sectionInfo__)
{
    int ptr = sectionInfo__.SubAddr;
    for (int i = 0; i < sectionInfo__.SubCount; i++)
    {
        sectionSubInfo sectionSubInfo__;
        sectionSubInfo__.ItemAddr = ptr;
        sectionSubInfo__.Addr = sectionInfo__.Addr;
        sectionSubInfo__.Index = i;
        ptr++;

        int vecLen = leb128u(ptr);
        ptr += leb128Size;
        for (int ii = 0; ii < vecLen; ii++)
        {
            sectionSubInfo__._TypeParams.push_back(raw[ptr]);
            ptr++;
        }
        vecLen = leb128u(ptr);
        ptr += leb128Size;
        for (int ii = 0; ii < vecLen; ii++)
        {
            sectionSubInfo__._TypeReturn.push_back(raw[ptr]);
            ptr++;
        }

        sectionSubInfo__.ItemSize = ptr - sectionSubInfo__.ItemAddr;
        sectionSubInfo_.push_back(sectionSubInfo__);
    }
    sectionInfo__.ParseStatus = (sectionInfo__.Size == (ptr - sectionInfo__.Addr)) ? 1 : 0;
}

void fileStructure::parseFunction(sectionInfo &sectionInfo__)
{
    int ptr = sectionInfo__.SubAddr;
    for (int i = 0; i < sectionInfo__.SubCount; i++)
    {
        sectionSubInfo sectionSubInfo__;
        sectionSubInfo__.ItemAddr = ptr;
        sectionSubInfo__.Addr = sectionInfo__.Addr;
        sectionSubInfo__.Index = i;
        sectionSubInfo__._FunctionIdx = leb128u(ptr);
        ptr += leb128Size;
        sectionSubInfo__.ItemSize = ptr - sectionSubInfo__.ItemAddr;
        sectionSubInfo_.push_back(sectionSubInfo__);
    }
    sectionInfo__.ParseStatus = (sectionInfo__.Size == (ptr - sectionInfo__.Addr)) ? 1 : 0;
}

void fileStructure::parseExport(sectionInfo &sectionInfo__)
{
    int ptr = sectionInfo__.SubAddr;
    for (int i = 0; i < sectionInfo__.SubCount; i++)
    {
        sectionSubInfo sectionSubInfo__;
        sectionSubInfo__.Addr = sectionInfo__.Addr;
        sectionSubInfo__.Index = i;


        sectionSubInfo__.ItemAddr = ptr;
        int vecLen = leb128u(ptr);
        ptr += leb128Size;
        sectionSubInfo__._FunctionName = "";
        while (vecLen > 0)
        {
            sectionSubInfo__._FunctionName.push_back(raw[ptr]);
            ptr++;
            vecLen--;
        }
        sectionSubInfo__._FunctionTag = raw[ptr];
        ptr++;
        sectionSubInfo__._FunctionIdx = leb128u(ptr);
        ptr += leb128Size;
        sectionSubInfo__.ItemSize = ptr - sectionSubInfo__.ItemAddr;
        sectionSubInfo_.push_back(sectionSubInfo__);
    }
    sectionInfo__.ParseStatus = (sectionInfo__.Size == (ptr - sectionInfo__.Addr)) ? 1 : 0;
}

void fileStructure::parseImport(sectionInfo &sectionInfo__)
{
    int ptr = sectionInfo__.SubAddr;
    for (int i = 0; i < sectionInfo__.SubCount; i++)
    {
        sectionSubInfo sectionSubInfo__;
        sectionSubInfo__.Addr = sectionInfo__.Addr;
        sectionSubInfo__.Index = i;

        sectionSubInfo__.ItemAddr = ptr;
        int vecLen = leb128u(ptr);
        ptr += leb128Size;
        std::string ModName = "";
        while (vecLen > 0)
        {
            ModName.push_back(raw[ptr]);
            ptr++;
            vecLen--;
        }

        vecLen = leb128u(ptr);
        ptr += leb128Size;
        sectionSubInfo__._FunctionName = "";
        while (vecLen > 0)
        {
            sectionSubInfo__._FunctionName.push_back(raw[ptr]);
            ptr++;
            vecLen--;
        }

        if ((ModName.size() > 0))
        {
            if (sectionSubInfo__._FunctionName.size() > 0)
            {
                sectionSubInfo__._FunctionName = ModName + "." + sectionSubInfo__._FunctionName;
            }
            else
            {
                sectionSubInfo__._FunctionName = ModName;
            }
        }

        sectionSubInfo__._FunctionTag = raw[ptr];
        int memLimited;
        switch (sectionSubInfo__._FunctionTag)
        {
            case 0x00: // function
                ptr++;
                sectionSubInfo__._CodeSize = leb128u(ptr);
                ptr += leb128Size;
                sectionSubInfo__._FunctionIdx = parseFunctionId;
                parseFunctionId++;
                break;
            case 0x01: // table
                ptr++;
                // 0x70 = funcref
                // 0x6f = externref
                ptr++;
                if (raw[ptr] == 0) ptr += 2;
                if (raw[ptr] == 1) ptr += 3;
                sectionSubInfo__._FunctionIdx = parseTableId;
                parseTableId++;
                break;
            case 0x02: // memory
                ptr++;

                // Index
                memLimited = leb128u(ptr);
                ptr += leb128Size;

                // Initial
                sectionSubInfo__._TypeReturn.push_back(leb128u(ptr));
                ptr += leb128Size;

                // Maximum
                if (memLimited)
                {
                    sectionSubInfo__._TypeReturn.push_back(leb128u(ptr));
                    ptr += leb128Size;
                }
                else
                {
                    sectionSubInfo__._TypeReturn.push_back(-1);
                }

                sectionSubInfo__._FunctionIdx = parseMemoId;
                parseMemoId++;
                break;
            case 0x03: // global
                ptr++;
                // global type byte
                sectionSubInfo__._TypeReturn.push_back(raw[ptr]);
                ptr++;
                // 0x00 = const
                // 0x01 = var
                sectionSubInfo__._TypeReturn.push_back(raw[ptr]);
                ptr++;
                sectionSubInfo__._FunctionIdx = parseGlobalId;
                parseGlobalId++;
                break;
        }

        sectionSubInfo__.ItemSize = ptr - sectionSubInfo__.ItemAddr;
        sectionSubInfo_.push_back(sectionSubInfo__);

    }
    sectionInfo__.ParseStatus = (sectionInfo__.Size == (ptr - sectionInfo__.Addr)) ? 1 : 0;
}

void fileStructure::parseGlobal(sectionInfo &sectionInfo__)
{
    int ptr = sectionInfo__.SubAddr;
    for (int i = 0; i < sectionInfo__.SubCount; i++)
    {
        sectionSubInfo sectionSubInfo__;
        sectionSubInfo__.ItemAddr = ptr;
        sectionSubInfo__.Addr = ptr;
        sectionSubInfo__.Index = i;
        sectionSubInfo__._TypeReturn.push_back(raw[ptr]);
        sectionSubInfo__._TypeReturn.push_back(raw[ptr + 1]);
        ptr++;
        ptr++;

        parseInstructions(ptr, sectionSubInfo__);

        // Dummy set global instruction
        /*codeInstr codeInstr_;
        codeInstr_.Addr = -1;
        codeInstr_.Size = 0;
        codeInstr_.Depth = 0;
        codeInstr_.Opcode = 0x24;
        codeInstr_.Param0 = std::to_string(sectionSubInfo__.Index);
        sectionSubInfo__._CodeInstr.push_back(codeInstr_);*/

        for (int ii = 0; ii < sectionSubInfo__._CodeInstr.size(); ii++)
        {
            ptr += sectionSubInfo__._CodeInstr[ii].Size;
        }

        sectionSubInfo__._FunctionIdx = parseGlobalId;
        parseGlobalId++;

        sectionSubInfo__._CodeGood = isCodeGood(sectionSubInfo__);
        sectionSubInfo__.ItemSize = ptr - sectionSubInfo__.ItemAddr;
        sectionSubInfo_.push_back(sectionSubInfo__);
    }
    sectionInfo__.ParseStatus = (sectionInfo__.Size == (ptr - sectionInfo__.Addr)) ? 1 : 0;
}

void fileStructure::parseData(sectionInfo &sectionInfo__)
{
    int ptr = sectionInfo__.SubAddr;
    for (int i = 0; i < sectionInfo__.SubCount; i++)
    {
        sectionSubInfo sectionSubInfo__;
        sectionSubInfo__.ItemAddr = ptr;
        sectionSubInfo__.Addr = ptr;
        sectionSubInfo__.Index = i;

        // Memory index
        int MemIdx = leb128u(ptr);
        ptr += leb128Size;

        // Instructions
        if (MemIdx == 0)
        {
            parseInstructions(ptr, sectionSubInfo__);
            for (int ii = 0; ii < sectionSubInfo__._CodeInstr.size(); ii++)
            {
                ptr += sectionSubInfo__._CodeInstr[ii].Size;
            }
        }

        // Data
        int vecSize = leb128u(ptr);
        ptr += leb128Size;
        sectionSubInfo__._CodeAddr = ptr;
        sectionSubInfo__._CodeSize = vecSize;
        ptr += vecSize;

        sectionSubInfo__._CodeGood = isCodeGood(sectionSubInfo__);
        //ptr += sectionSubInfo__._CodeSize;
        ptr = sectionSubInfo__._CodeAddr + sectionSubInfo__._CodeSize;


        sectionSubInfo__.ItemSize = ptr - sectionSubInfo__.ItemAddr;
        sectionSubInfo_.push_back(sectionSubInfo__);
    }
    sectionInfo__.ParseStatus = (sectionInfo__.Size == (ptr - sectionInfo__.Addr)) ? 1 : 0;
}

void fileStructure::parseCode(sectionInfo &sectionInfo__)
{
    int ptr = sectionInfo__.SubAddr;
    for (int i = 0; i < sectionInfo__.SubCount; i++)
    {
        sectionSubInfo sectionSubInfo__;
        sectionSubInfo__.ItemAddr = ptr;
        sectionSubInfo__._CodeGood = true;
        sectionSubInfo__.Addr = sectionInfo__.Addr;
        sectionSubInfo__.Index = i;
        sectionSubInfo__._FunctionIdx = parseFunctionId;
        parseFunctionId++;
        sectionSubInfo__._CodeSize = leb128u(ptr);
        ptr += leb128Size;
        sectionSubInfo__._CodeAddr = ptr;

        int ptr_ = ptr;
        int vecLen = leb128u(ptr_);
        ptr_ += leb128Size;
        sectionSubInfo__._CodeAddr_ = ptr_;
        for (int ii = 0; ii < vecLen; ii++)
        {
            sectionSubInfo__._CodeLocalAddr.push_back(ptr_);
            sectionSubInfo__._CodeLocalN.push_back(leb128u(ptr_));
            ptr_ += leb128Size;
            int s = leb128Size + 1;
            sectionSubInfo__._CodeLocalType.push_back(raw[ptr_]);
            ptr_++;
            sectionSubInfo__._CodeLocalSize.push_back(s);
        }

        int codeLen = parseInstructions(ptr_, sectionSubInfo__);
        int codeLen0 = sectionSubInfo__._CodeSize - (ptr_ - ptr);
        sectionSubInfo__._CodeGood = ((codeLen == codeLen0) && isCodeGood(sectionSubInfo__));

        ptr += sectionSubInfo__._CodeSize;
        sectionSubInfo__.ItemSize = ptr - sectionSubInfo__.ItemAddr;
        sectionSubInfo_.push_back(sectionSubInfo__);
    }
    sectionInfo__.ParseStatus = (sectionInfo__.Size == (ptr - sectionInfo__.Addr)) ? 1 : 0;
}


void fileStructure::parseStart(sectionInfo &sectionInfo__)
{
    int ptr = sectionInfo__.DataAddr;
    sectionInfo__.StartIdx = leb128u(ptr);
    ptr += leb128Size;
    sectionInfo__.ParseStatus = (sectionInfo__.Size == (ptr - sectionInfo__.Addr)) ? 1 : 0;
}

void fileStructure::parseDataCount(sectionInfo &sectionInfo__)
{
    sectionInfo__.ParseStatus = 2;
}

void fileStructure::parseTable(sectionInfo &sectionInfo__)
{
    sectionInfo__.ParseStatus = 2;
}

void fileStructure::parseMemory(sectionInfo &sectionInfo__)
{
    int ptr = sectionInfo__.SubAddr;
    for (int i = 0; i < sectionInfo__.SubCount; i++)
    {
        sectionSubInfo sectionSubInfo__;
        sectionSubInfo__.ItemAddr = ptr;
        sectionSubInfo__.Addr = ptr;
        sectionSubInfo__.Index = i;

        // 0 - min-unlimited
        // 1 - min-max
        int memLimited = raw[ptr];
        ptr++;

        // Min
        sectionSubInfo__._CodeLocalSize.push_back(leb128u(ptr));
        ptr += leb128Size;

        // Max
        if (memLimited == 1)
        {
            sectionSubInfo__._CodeLocalSize.push_back(leb128u(ptr));
            ptr += leb128Size;
        }
        else
        {
            sectionSubInfo__._CodeLocalSize.push_back(-1);
        }

        sectionSubInfo__.ItemSize = ptr - sectionSubInfo__.ItemAddr;
        sectionSubInfo_.push_back(sectionSubInfo__);
    }
    sectionInfo__.ParseStatus = (sectionInfo__.Size == (ptr - sectionInfo__.Addr)) ? 1 : 0;
}

void fileStructure::parseElement(sectionInfo &sectionInfo__)
{
    sectionInfo__.ParseStatus = 2;
}


void fileStructure::parse(uchar * raw_, int rawSize_)
{
    raw = raw_;
    rawSize = rawSize_;
    sectionInfo_.clear();
    sectionSubInfo_.clear();
    if (rawSize < 8)
    {
        return;
    }

    sectionInfo sectionInfo__;
    sectionInfo__.Id = 255;
    sectionInfo__.ParseStatus = 1;
    sectionInfo__.Addr = 0;
    sectionInfo__.Size = 8;
    sectionInfo__.DataAddr = 0;
    sectionInfo__.DataSize = 8;
    sectionInfo_.push_back(sectionInfo__);
    int raw_ptr = 8;
    parseFunctionId = 0;
    parseTableId = 0;
    parseMemoId = 0;
    parseGlobalId = 0;
    while (raw_ptr < rawSize)
    {
        int S = leb128u(raw_ptr + 1);
        sectionInfo__.Id = raw[raw_ptr];
        sectionInfo__.Addr = raw_ptr;
        sectionInfo__.Size = S + leb128Size + 1;
        sectionInfo__.DataAddr = raw_ptr + leb128Size + 1;
        sectionInfo__.DataSize = S;

        sectionInfo__.SubAddr = sectionInfo__.DataAddr;
        sectionInfo__.SubCount = 0;
        int raw_ptr_ = sectionInfo__.DataAddr;
        raw_ptr = raw_ptr + S + leb128Size + 1;
        switch (sectionInfo__.Id)
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
                {
                    int SectionSubCount = leb128u(raw_ptr_);
                    raw_ptr_ += leb128Size;
                    sectionInfo__.SubCount = SectionSubCount;
                    sectionInfo__.SubAddr = raw_ptr_;
                    sectionInfo__.SubIdx1 = sectionSubInfo_.size();
                    sectionInfo__.SubIdx2 = sectionSubInfo_.size() + SectionSubCount;
                }

                break;
        }

        switch (sectionInfo__.Id)
        {
            case 1: parseType(sectionInfo__); break;
            case 2: parseImport(sectionInfo__); break;
            case 3: parseFunction(sectionInfo__); break;
            case 4: parseTable(sectionInfo__); break;
            case 5: parseMemory(sectionInfo__); break;
            case 6: parseGlobal(sectionInfo__); break;
            case 7: parseExport(sectionInfo__); break;
            case 8: parseStart(sectionInfo__); break;
            case 9: parseElement(sectionInfo__); break;
            case 10: parseCode(sectionInfo__); break;
            case 11: parseData(sectionInfo__); break;
            case 12: parseDataCount(sectionInfo__); break;
        }

        sectionInfo_.push_back(sectionInfo__);
    }
}
