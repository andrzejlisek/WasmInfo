#include "wasmdecompilerindentdata.h"

wasmDecompilerIndentData::wasmDecompilerIndentData()
{
    instr = 0;
}

std::string wasmDecompilerIndentData::instrString()
{
    switch (instr)
    {
        case 0x02: // block
            return "block";
        case 0x03: // loop
            return "loop";
        case 0x06: // try
            return "try";
        case 0x04: // if
            return "if";
        case 0x05: // else
            return "else";
        case 0x07: // catch
            return "catch";
        case 0x19: // catch_all
            return "catch_all";
        case 0x0B: // end
            return "end";
        default:
            return "[" + std::to_string(instr) + "]";
    }
}
