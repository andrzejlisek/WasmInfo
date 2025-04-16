#include "filestructure.h"

void fileStructure::parseCustom(sectionInfo &sectionInfo__, int idx)
{
    int ptr = sectionInfo__.DataAddr;
    sectionInfo__.StartIdx = leb128u(ptr) * 100;
    sectionInfo__.StartIdx += leb128Size;
    ptr += leb128Size;
    int nameLength = (sectionInfo__.StartIdx / 100);
    int nameNumSie = (sectionInfo__.StartIdx % 100);

    bool isName = false;
    if ((nameLength == 4) && (nameNumSie == 1))
    {
        if ((raw[ptr + 0] == 'n') && (raw[ptr + 1] == 'a') && (raw[ptr + 2] == 'm') && (raw[ptr + 3] == 'e'))
        {
            isName = true;
        }
    }

    sectionSubInfo sectionSubInfo__;
    sectionSubInfo__.ItemAddr = sectionInfo__.DataAddr;
    sectionSubInfo__.Addr = sectionInfo__.Addr;
    sectionSubInfo__.Index = idx;
    sectionSubInfo__.ItemSize = sectionInfo__.DataSize;
    sectionSubInfo__._CodeAddr = sectionInfo__.DataAddr + nameNumSie + nameLength;
    sectionSubInfo__._CodeSize = sectionInfo__.DataSize - nameNumSie - nameLength;
    ptr += nameLength;


    if (isName && wasmDecompiler_.useTags)
    {
        int ptrMax = sectionInfo__.DataAddr + sectionSubInfo__.ItemSize;
        bool isValid = true;
        int lastItems = 0;
        while (ptr < ptrMax)
        {
            int metaType = raw[ptr];
            ptr++;
            int metaLength = leb128u(ptr);
            ptr += leb128Size;
            int ptrx = ptr;
            switch (metaType)
            {
                case 0: // Module
                    {
                        std::string metaName = leb128string(ptrx);
                        ptrx += leb128Size;
                        if (isValid)
                        {
                            wasmDecompiler_.metaTagAdd(idx, 0, 0, 0, metaName);
                            lastItems++;
                        }
                    }
                    break;
                case 1: // Function
                case 4: // Type
                case 5: // Table
                case 6: // Memory
                case 7: // Global
                case 8: // Element
                case 9: // Data
                case 11: // Tag
                    {
                        int metaListLength = leb128u(ptrx);
                        ptrx += leb128Size;
                        for (int i = 0; i < metaListLength; i++)
                        {
                            int metaItemIdx = leb128u(ptrx);
                            ptrx += leb128Size;
                            std::string metaItemStr = leb128string(ptrx);
                            ptrx += leb128Size;
                            if (isValid)
                            {
                                wasmDecompiler_.metaTagAdd(idx, metaType, metaItemIdx, 0, metaItemStr);
                                lastItems++;
                            }
                        }
                    }
                    break;
                case 2: // Local
                    {
                        int metaListLength = leb128u(ptrx);
                        ptrx += leb128Size;
                        for (int i = 0; i < metaListLength; i++)
                        {
                            int metaFunctionIdx = leb128u(ptrx);
                            ptrx += leb128Size;

                            int metaLocalCount = leb128u(ptrx);
                            ptrx += leb128Size;
                            for (int ii = 0; ii < metaLocalCount; ii++)
                            {
                                int metaVarIdx = leb128u(ptrx);
                                ptrx += leb128Size;

                                std::string metaVarStr = leb128string(ptrx);
                                ptrx += leb128Size;
                                if (isValid)
                                {
                                    wasmDecompiler_.metaTagAdd(idx, metaType, metaFunctionIdx, metaVarIdx, metaVarStr);
                                    lastItems++;
                                }
                            }
                        }
                    }
                    break;
                case 3: // Label
                    {
                        ptr += metaLength;
                    }
                    break;
                case 10: // Field
                    {
                        ptr += metaLength;
                    }
                    break;
                default:
                    {
                        isValid = false;
                    }
                    break;
            }
            if (ptrx != (ptr + metaLength))
            {
                isValid = false;
            }

            ptr += metaLength;
            if (ptr > ptrMax)
            {
                isValid = false;
            }
        }
        if (!isValid)
        {
            wasmDecompiler_.metaTagRemoveLast(lastItems);
        }
    }

    sectionSubInfo_.push_back(sectionSubInfo__);

    sectionInfo__.ParseStatus = 1;
}

void fileStructure::parseType(sectionInfo &sectionInfo__)
{
    int ptr = sectionInfo__.SubAddr;
    for (int i = 0; i < sectionInfo__.SubCount; i++)
    {
        sectionSubInfo sectionSubInfo__;
        sectionSubInfo__.ItemAddr = ptr;
        sectionSubInfo__.Addr = sectionInfo__.Addr;
        sectionSubInfo__.Index = i;

        switch (raw[ptr])
        {
            case 0x60: // functype
                {
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
                }
                break;
            case 0x5F: // structtype
                {
                    ptr++;

                    int vecLen = leb128u(ptr);
                    ptr += leb128Size;

                    sectionSubInfo__._TypeParams.push_back(0);
                    sectionSubInfo__._TypeParams.push_back(wasmDecompiler_.valueTypeNumber("structref"));

                    for (int ii = 0; ii < vecLen; ii++)
                    {
                        int t = raw[ptr];
                        ptr++;

                        if (raw[ptr] == 1)
                        {
                            sectionSubInfo__._TypeReturn.push_back(t);
                        }
                        else
                        {
                            sectionSubInfo__._TypeReturn.push_back(256 + t);
                        }
                        ptr++;
                    }
                }
                break;
            case 0x5E: // arraytype
                {
                    ptr++;
                    sectionSubInfo__._TypeParams.push_back(0);
                    sectionSubInfo__._TypeParams.push_back(wasmDecompiler_.valueTypeNumber("arrayref"));

                    int t = raw[ptr];
                    ptr++;

                    if (raw[ptr] == 1)
                    {
                        sectionSubInfo__._TypeReturn.push_back(t);
                    }
                    else
                    {
                        sectionSubInfo__._TypeReturn.push_back(256 + t);
                    }
                    ptr++;
                }
                break;
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
        switch (sectionSubInfo__._FunctionTag)
        {
            case 0: // Function
                wasmDecompiler_.metaTagAdd(-1, 201, sectionSubInfo__._FunctionIdx, 0, sectionSubInfo__._FunctionName);
                break;
            case 1: // Table
                wasmDecompiler_.metaTagAdd(-1, 205, sectionSubInfo__._FunctionIdx, 0, sectionSubInfo__._FunctionName);
                break;
            case 2: // Memory
                wasmDecompiler_.metaTagAdd(-1, 206, sectionSubInfo__._FunctionIdx, 0, sectionSubInfo__._FunctionName);
                break;
            case 3: // Global
                wasmDecompiler_.metaTagAdd(-1, 207, sectionSubInfo__._FunctionIdx, 0, sectionSubInfo__._FunctionName);
                break;
        }

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

        if ((!ModName.empty()))
        {
            if (!sectionSubInfo__._FunctionName.empty())
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
                wasmDecompiler_.metaTagAdd(-1, 101, sectionSubInfo__._FunctionIdx, 0, sectionSubInfo__._FunctionName);
                parseFunctionId++;
                break;
            case 0x01: // table
                ptr++;
                sectionSubInfo__._TypeReturn.push_back(raw[ptr]);
                ptr++;
                if (raw[ptr] == 1)
                {
                    ptr++;
                    sectionSubInfo__._TypeReturn.push_back(leb128u(ptr));
                    ptr += leb128Size;
                    sectionSubInfo__._TypeReturn.push_back(leb128u(ptr));
                    ptr += leb128Size;
                }
                else
                {
                    ptr++;
                    sectionSubInfo__._TypeReturn.push_back(leb128u(ptr));
                    ptr += leb128Size;
                    sectionSubInfo__._TypeReturn.push_back(-1);
                }
                sectionSubInfo__._FunctionIdx = parseTableId;
                wasmDecompiler_.metaTagAdd(-1, 105, sectionSubInfo__._FunctionIdx, 0, sectionSubInfo__._FunctionName);
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
                wasmDecompiler_.metaTagAdd(-1, 106, sectionSubInfo__._FunctionIdx, 0, sectionSubInfo__._FunctionName);
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
                wasmDecompiler_.metaTagAdd(-1, 107, sectionSubInfo__._FunctionIdx, 0, sectionSubInfo__._FunctionName);
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

        parseInstructions(ptr, sectionSubInfo__, sectionSubInfo_.size(), 2);

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
            parseInstructions(ptr, sectionSubInfo__, sectionSubInfo_.size(), 3);
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

    // Create stubs
    int parseFunctionId_ = parseFunctionId;
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


        ptr += sectionSubInfo__._CodeSize;
        sectionSubInfo__.ItemSize = ptr - sectionSubInfo__.ItemAddr;
        sectionSubInfo_.push_back(sectionSubInfo__);
    }
    parseFunctionId = parseFunctionId_;
}

void fileStructure::parseCode2(sectionInfo &sectionInfo__)
{
    int sectionSubInfoIdx = sectionSubInfo_.size() - sectionInfo__.SubCount;

    int ptr = sectionInfo__.SubAddr;
    for (int i = 0; i < sectionInfo__.SubCount; i++)
    {
        sectionSubInfo sectionSubInfo__ = sectionSubInfo_[sectionSubInfoIdx + i];
        parseFunctionId++;
        leb128u(ptr);
        ptr += leb128Size;


        int ptr_ = ptr;
        int vecLen = leb128u(ptr_);
        ptr_ += leb128Size;
        sectionSubInfo__._CodeAddr_ = ptr_;
        for (int ii = 0; ii < vecLen; ii++)
        {
            leb128u(ptr_);
            ptr_ += leb128Size;
            ptr_++;
        }


        int codeLen = parseInstructions(ptr_, sectionSubInfo__, sectionSubInfo_.size(), 1);
        int codeLen0 = sectionSubInfo__._CodeSize - (ptr_ - ptr);
        sectionSubInfo__._CodeGood = ((codeLen == codeLen0) && isCodeGood(sectionSubInfo__));

        ptr += sectionSubInfo__._CodeSize;
        sectionSubInfo_[sectionSubInfoIdx + i] = sectionSubInfo__;
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
    int ptr = sectionInfo__.SubAddr;
    sectionInfo__.StartIdx = leb128u(ptr);
    ptr += leb128Size;
    sectionInfo__.ParseStatus = (sectionInfo__.Size == (ptr - sectionInfo__.Addr)) ? 1 : 0;
}

void fileStructure::parseTag(sectionInfo &sectionInfo__)
{
    int ptr = sectionInfo__.SubAddr;
    for (int i = 0; i < sectionInfo__.SubCount; i++)
    {
        sectionSubInfo sectionSubInfo__;
        sectionSubInfo__.ItemAddr = ptr;
        sectionSubInfo__.Addr = ptr;
        sectionSubInfo__.Index = i;
        sectionSubInfo__._TypeReturn.push_back(leb128u(ptr));
        ptr += leb128Size;
        sectionSubInfo__._TypeReturn.push_back(leb128u(ptr));
        ptr += leb128Size;

        sectionSubInfo__.ItemSize = ptr - sectionSubInfo__.ItemAddr;
        sectionSubInfo_.push_back(sectionSubInfo__);
    }
    sectionInfo__.ParseStatus = (sectionInfo__.Size == (ptr - sectionInfo__.Addr)) ? 1 : 0;
}

void fileStructure::parseTable(sectionInfo &sectionInfo__)
{
    int ptr = sectionInfo__.SubAddr;
    for (int i = 0; i < sectionInfo__.SubCount; i++)
    {
        sectionSubInfo sectionSubInfo__;
        sectionSubInfo__.ItemAddr = ptr;
        sectionSubInfo__.Addr = sectionInfo__.Addr;
        sectionSubInfo__.Index = i;

        // Data type
        sectionSubInfo__._TypeReturn.push_back(raw[ptr]);
        ptr++;

        // Limit
        if (raw[ptr] == 1)
        {
            ptr++;
            sectionSubInfo__._CodeLocalSize.push_back(leb128u(ptr));
            ptr += leb128Size;
            sectionSubInfo__._CodeLocalSize.push_back(leb128u(ptr));
            ptr += leb128Size;
        }
        else
        {
            ptr++;
            sectionSubInfo__._CodeLocalSize.push_back(leb128u(ptr));
            ptr += leb128Size;
            sectionSubInfo__._CodeLocalSize.push_back(-1);
        }

        sectionSubInfo__._FunctionIdx = parseTableId;
        sectionSubInfo__.ItemSize = ptr - sectionSubInfo__.ItemAddr;
        sectionSubInfo_.push_back(sectionSubInfo__);

        parseTableId++;
    }
    sectionInfo__.ParseStatus = (sectionInfo__.Size == (ptr - sectionInfo__.Addr)) ? 1 : 0;
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

        sectionSubInfo__._FunctionIdx = parseMemoId;
        sectionSubInfo__.ItemSize = ptr - sectionSubInfo__.ItemAddr;
        sectionSubInfo_.push_back(sectionSubInfo__);

        parseMemoId++;
    }
    sectionInfo__.ParseStatus = (sectionInfo__.Size == (ptr - sectionInfo__.Addr)) ? 1 : 0;
}

void fileStructure::parseElement(sectionInfo &sectionInfo__)
{
    int ptr = sectionInfo__.SubAddr;
    for (int i = 0; i < sectionInfo__.SubCount; i++)
    {
        sectionSubInfo sectionSubInfo__;
        sectionSubInfo__.ItemAddr = ptr;
        sectionSubInfo__.Addr = ptr;
        sectionSubInfo__.Index = i;

        int segmentType = raw[ptr];

        sectionSubInfo__._FunctionTag = segmentType;

        ptr++;

        switch (segmentType)
        {
            case 0: // active
            case 2: // active with table index
                {
                    if (segmentType == 2)
                    {
                        // Table number
                        sectionSubInfo__._TypeParams.push_back(leb128u(ptr));
                        ptr += leb128Size;
                    }
                    else
                    {
                        sectionSubInfo__._TypeParams.push_back(0);
                    }

                    parseInstructions(ptr, sectionSubInfo__, sectionSubInfo_.size(), 3);
                    for (int ii = 0; ii < sectionSubInfo__._CodeInstr.size(); ii++)
                    {
                        ptr += sectionSubInfo__._CodeInstr[ii].Size;
                    }

                    if (segmentType == 2)
                    {
                        ptr++;
                    }
                }
                break;
            case 1: // passive
            case 3: // declare
                {
                    // Dummy table number
                    sectionSubInfo__._TypeParams.push_back(0);

                    // ???
                    ptr++;
                }
                break;
        }

        switch (segmentType)
        {
            case 0:
            case 1:
            case 2:
            case 3:
                {
                    int funcVector = leb128u(ptr);
                    ptr += leb128Size;
                    for (int ii = 0; ii < funcVector; ii++)
                    {
                        sectionSubInfo__._TypeParams.push_back(ptr);
                        sectionSubInfo__._TypeParams.push_back(leb128u(ptr));
                        ptr += leb128Size;
                    }
                    sectionSubInfo__._TypeParams.push_back(ptr);
                }
                break;
        }





        sectionSubInfo__.ItemSize = ptr - sectionSubInfo__.ItemAddr;
        sectionSubInfo_.push_back(sectionSubInfo__);
    }
    sectionInfo__.ParseStatus = (sectionInfo__.Size == (ptr - sectionInfo__.Addr)) ? 1 : 0;
}


void fileStructure::parse(uchar * raw_, int rawSize_)
{
    if (rawSize_ < 0)
    {
        rawSize_ = 0 - rawSize_;
        wasmDecompiler_.useHtml = false;
    }

    wasmDecompiler_.metaTagClear();
    wasmDecompiler_.codeInstrInfoLength = 0;
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
    parseFunctionId = 0;
    parseTableId = 0;
    parseMemoId = 0;
    parseGlobalId = 0;

    int raw_ptr = 8;
    int customIdx = -1;
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
            case 0:
                {
                    // Fake one subsection for "Custom" section
                    customIdx++;
                    int SectionSubCount = 1;
                    sectionInfo__.SubCount = SectionSubCount;
                    sectionInfo__.SubAddr = raw_ptr_;
                    sectionInfo__.SubIdx1 = sectionSubInfo_.size();
                    sectionInfo__.SubIdx2 = sectionSubInfo_.size() + SectionSubCount;
                }
                break;
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
            case 13:
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
            case 0: parseCustom(sectionInfo__, customIdx); break;
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
            case 13: parseTag(sectionInfo__); break;
        }

        sectionInfo_.push_back(sectionInfo__);

        if (sectionInfo__.Id == 10)
        {
            parseCode2(sectionInfo__);
            sectionInfo_[sectionInfo_.size() - 1] = sectionInfo__;
        }
    }

    wasmDecompiler_.metaTagValidateNames();
}
