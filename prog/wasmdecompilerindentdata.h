#ifndef WASMDECOMPILERINDENTDATA_H
#define WASMDECOMPILERINDENTDATA_H

#include <string>

class wasmDecompilerIndentData
{
public:
    wasmDecompilerIndentData();
    int instr;
    std::string debugInfo();
    std::string instrString();
};

#endif // WASMDECOMPILERINDENTDATA_H
