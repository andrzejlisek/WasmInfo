#include "wasmdecompiler.h"

wasmDecompiler::wasmDecompiler()
{
    // MVP
    valueTypeXNumber.push_back(0x7F); valueTypeXName.push_back("i32"); // int
    valueTypeXNumber.push_back(0x7E); valueTypeXName.push_back("i64"); // long
    valueTypeXNumber.push_back(0x7D); valueTypeXName.push_back("f32"); // float
    valueTypeXNumber.push_back(0x7C); valueTypeXName.push_back("f64"); // double

    // SIMD
    valueTypeXNumber.push_back(0x7B); valueTypeXName.push_back("v128"); // __m128

    // Reference type
    valueTypeXNumber.push_back(0x70); valueTypeXName.push_back("funcref");
    valueTypeXNumber.push_back(0x6F); valueTypeXName.push_back("externref");

    // Garbage Collection
    valueTypeXNumber.push_back(0x6E); valueTypeXName.push_back("anyref"); //externref,eqref,i31ref,structref,arrayref
    valueTypeXNumber.push_back(0x6D); valueTypeXName.push_back("eqref");
    valueTypeXNumber.push_back(0x6C); valueTypeXName.push_back("i31ref");
    valueTypeXNumber.push_back(0x6B); valueTypeXName.push_back("structref");
    valueTypeXNumber.push_back(0x6A); valueTypeXName.push_back("arrayref");
    valueTypeXNumber.push_back(0x69); valueTypeXName.push_back("exnref");
    valueTypeXNumber.push_back(0x68); valueTypeXName.push_back("ref");
    valueTypeXNumber.push_back(0x67); valueTypeXName.push_back("refnull");
}

int wasmDecompiler::valueTypeNumber(std::string typeSig)
{
    for (int i = 0; i < valueTypeXNumber.size(); i++)
    {
        if (valueTypeXName[i] == typeSig) return valueTypeXNumber[i];
    }
    return 0x00;
}

bool wasmDecompiler::valueTypeIsStandard(int typeSig)
{
    if (typeSig >= 0x40)
    {
        return true;

    }
    else
    {
        return false;
    }

    for (int i = 0; i < valueTypeXNumber.size(); i++)
    {
        if (valueTypeXNumber[i] == typeSig) return true;
    }
    return false;
}

std::string wasmDecompiler::valueTypeName(int typeSig)
{
    if (typeSig >= 256)
    {
        return "const " + valueTypeName(typeSig - 256);
    }

    for (int i = 0; i < valueTypeXNumber.size(); i++)
    {
        if (valueTypeXNumber[i] == typeSig) return valueTypeXName[i];
    }

    switch (typeSig)
    {
        case 0x00: return "value";
        case fieldType_u_: return "u";
        case fieldType_void: return "void";
        default:   return "0x" + hex::IntToHex8(typeSig);
    }
}

bool wasmDecompiler::instrBlockBegin(std::string txt)
{
    return (txt[txt.size() - 1] == '{');
}

bool wasmDecompiler::instrBlockEnd(std::string txt)
{
    return (txt[0] == '}');
}

void wasmDecompiler::reset(std::string funcName_, int decompBranch_, std::vector<dataField2> dataFieldDictionary_)
{
    decompBranch = decompBranch_;
    lastOpcode = 0;
    funcName = funcName_;
    dataFieldDictionary.clear();

    for (int i = 0; i < dataFieldDictionary_.size(); i++)
    {
        dataFieldDictionary.push_back(dataFieldDictionary_[i]);
    }

    WDF.depth = 0;
    WDF.isFoldable = false;
    WDF.name = "root";
    WDF.returnName = "root";
    WDF.params.clear();

    currentDepth = 1;
    currentStack = 0;
    currentStack_.clear();
    indentStack.clear();
    indentStack.push_back(wasmDecompilerIndentData());
}

std::string wasmDecompiler::dataFieldDictionaryGetVar(std::string category, int num)
{
    if (category == "param")
    {
        category = "local";
    }
    for (int i = (dataFieldDictionary.size() - 1); i >= 0; i--)
    {
        if ((dataFieldDictionary[i].fieldCategory == category) && (dataFieldDictionary[i].fieldNumber == num))
        {
            return dataFieldDictionary[i].fieldId;
        }
    }
    return "ERROR[" + category + "_" + std::to_string(num) + "]";
}

std::string wasmDecompiler::dataFieldDictionaryGetConst(int type, std::string val)
{
    return "fld_" + valueTypeName(type & 255) + "_" + val + "_fld";
}

int wasmDecompiler::dataFieldDictionaryGetType(std::string category, int num)
{
    if (num < 0)
    {
        for (int i = (dataFieldDictionary.size() - 1); i >= 0; i--)
        {
            if (dataFieldDictionary[i].fieldId == category)
            {
                return dataFieldDictionary[i].fieldType;
            }
        }
    }
    else
    {
        for (int i = (dataFieldDictionary.size() - 1); i >= 0; i--)
        {
            if ((dataFieldDictionary[i].fieldCategory == category) && (dataFieldDictionary[i].fieldNumber == num))
            {
                return dataFieldDictionary[i].fieldType;
            }
        }
    }
    return 0;
}

int wasmDecompiler::dataFieldDictionaryIdx(std::string id)
{
    for (int i = (dataFieldDictionary.size() - 1); i >= 0; i--)
    {
        if (dataFieldDictionary[i].fieldId == id)
        {
            return i;
        }
    }
    return -1;
}

std::string wasmDecompiler::dataFieldDictionaryDisplay(std::string id)
{
    if (debugRawVariableNames)
    {
        return "<" + id.substr(3, id.size() - 6) + ">";
    }
    for (int i = (dataFieldDictionary.size() - 1); i >= 0; i--)
    {
        if (dataFieldDictionary[i].fieldId == id)
        {
            if (dataFieldDictionary[i].fieldCategory == "const")
            {
                std::string dispVal = dataFieldDictionary[i].fieldName;
                if (dataFieldDictionary[i].fieldType == fieldType_i64)
                {
                    dispVal = dispVal + "L";
                }
                if (dataFieldDictionary[i].fieldType == fieldType_f32)
                {
                    dispVal = dispVal + "f";
                }
                return dispVal;
            }
            else
            {
                std::string typePrefix = "";
                if (decompOptVariableHungarian)
                {
                    typePrefix = valueTypeName(dataFieldDictionary[i].fieldType & 255) + "_";
                }
                if (dataFieldDictionary[i].fieldName != "")
                {
                    return dataFieldDictionary[i].fieldName;
                }
                else
                {
                    if (dataFieldDictionary[i].isParam)
                    {
                        return typePrefix + metaTagGet2(102, metaTagFunctionNumber, dataFieldDictionary[i].fieldNumber, "param" + std::to_string(dataFieldDictionary[i].fieldNumber));
                    }
                    else
                    {
                        bool multiType = false;
                        if ((!decompOptVariableHungarian) && (dataFieldDictionary[i].fieldCategory == "stack"))
                        {
                            for (int ii = 0; ii < dataFieldDictionary.size(); ii++)
                            {
                                if (dataFieldDictionary[ii].fieldCategory == "stack")
                                {
                                    if (dataFieldDictionary[ii].fieldNumber == dataFieldDictionary[i].fieldNumber)
                                    {
                                        if (dataFieldDictionary[ii].fieldType != dataFieldDictionary[i].fieldType)
                                        {
                                            multiType = true;
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                        std::string varDispName = dataFieldDictionary[i].fieldCategory + std::to_string(dataFieldDictionary[i].fieldNumber);
                        if ((dataFieldDictionary[i].fieldCategory == "local") || (dataFieldDictionary[i].fieldCategory == "param"))
                        {
                            varDispName = metaTagGet2(102, metaTagFunctionNumber, dataFieldDictionary[i].fieldNumber, varDispName);
                        }
                        if ((dataFieldDictionary[i].fieldCategory == "global"))
                        {
                            varDispName = metaTagGet(107, dataFieldDictionary[i].fieldNumber, varDispName);
                        }
                        if (multiType)
                        {
                            return typePrefix + varDispName + "_" + valueTypeName(dataFieldDictionary[i].fieldType & 255);
                        }
                        else
                        {
                            return typePrefix + varDispName;
                        }
                    }
                }
            }
        }
    }
    return "ERROR[" + id + "]";
}

void wasmDecompiler::dataFieldDictionarySetType(std::string dataIdX, int dataTypeX)
{
    for (int i = 0; i < dataFieldDictionary.size(); i++)
    {
        if (dataFieldDictionary[i].fieldId == dataIdX)
        {
            if (dataTypeX > 0)
            {
                dataFieldDictionary[i].fieldType = dataTypeX;
            }
            return;
        }
    }
}

std::string wasmDecompiler::dataFieldDictionarySet(std::string dataNameX, int dataTypeX, std::string category, int num)
{
    dataField2 _;
    _.fieldName = dataNameX;
    _.fieldType = dataTypeX;
    _.fieldCategory = category;
    _.fieldNumber = num;
    _.isParam = false;
    if (category == "param")
    {
        _.isParam = true;
        _.fieldCategory = "local";
        category = "local";
    }

    if (category == "const")
    {
        _.fieldId = "fld_" + valueTypeName(dataTypeX & 255) + "_" + dataNameX + "_fld";
    }
    if ((category == "stack") || (category == "tempp") || (category == "tempr"))
    {
        //_.fieldId = "fld_" + std::to_string(dataFieldDictionary.size()) + "_fld";
        //_.fieldId = "fld_" + category + "_" + std::to_string(num) + "_" + std::to_string(dataFieldDictionary.size()) + "_fld";
        _.fieldId = "fld_" + category + "_" + std::to_string(num) + "_" + std::to_string(dataTypeX) + "_fld";
    }
    if ((category != "const") && (category != "stack") && (category != "tempp") && (category != "tempr"))
    {
        //_.fieldId = "fld_" + std::to_string(dataFieldDictionary.size()) + "_fld";
        _.fieldId = "fld_" + category + "_" + std::to_string(num) + "_fld";
    }

    for (int i = 0; i < dataFieldDictionary.size(); i++)
    {
        if (dataFieldDictionary[i].fieldId == _.fieldId)
        {
            if ((_.fieldName != "") && (!dataFieldDictionary[i].isParam))
            {
                dataFieldDictionary[i].fieldName = _.fieldName;
            }
            if (_.fieldType > 0)
            {
                dataFieldDictionary[i].fieldType = _.fieldType;
            }
            return _.fieldId;
        }
    }

    dataFieldDictionary.push_back(_);
    return _.fieldId;
}

bool wasmDecompiler::debugIsFunc(std::string funcName)
{
    int t = WDF.params[0].get()->name.find(funcName);
    return t > 0;
}

std::string wasmDecompiler::debugValues()
{
    std::string s;

    for (int I = 0; I < dataFieldDictionary.size(); I++)
    {
        if (dataFieldDictionary[I].fieldType >= 256)
        {
            s = s + dataFieldDictionary[I].fieldId + "=c" + valueTypeName(dataFieldDictionary[I].fieldType - 256) + "_" + dataFieldDictionary[I].fieldName + ", ";
        }
        else
        {
            s = s + dataFieldDictionary[I].fieldId + "=" + valueTypeName(dataFieldDictionary[I].fieldType) + "_" + dataFieldDictionary[I].fieldName + ", ";
        }
    }

    return s;
}

std::string wasmDecompiler::printCommand(int idx)
{
    if (idx == 0)
    {
        convertBlockToLabels();
        codeOptimize();

        // Prepare variable declaration
        localVarNamesD.clear();
        localVarNamesT.clear();
        for (int ii = 0; ii < dataFieldDictionary.size(); ii++)
        {
            if ((!dataFieldDictionary[ii].isParam) && (dataFieldDictionary[ii].fieldCategory == "global") || (dataFieldDictionary[ii].fieldCategory == "param"))
            {
                localVarNamesD.push_back(-1);
                localVarNamesT.push_back(dataFieldDictionaryDisplay(dataFieldDictionary[ii].fieldId));
            }
        }
    }

    std::string s = "";
    std::string ss = "";

    if (idx < WDF.params.size())
    {
        for (int ii = 0; ii < localVarNamesT.size(); ii++)
        {
            if (localVarNamesD[ii] > WDF.params[idx].get()->depth)
            {
                localVarNamesT[ii] = "";
            }
        }

        if (codeInstrInfoStack)
        {
            s = "[[^0^]]" + hex::indent(WDF.params[idx].get()->depth);
        }
        else
        {
            s = hex::indent(WDF.params[idx].get()->depth);
        }
        if (WDF.params[idx].get()->returnName != "")
        {
            if (WDF.params[idx].get()->returnNameItems.size() > 1)
            {
                s = s + "{";
                for (int i = 0; i < WDF.params[idx].get()->returnNameItems.size(); i++)
                {
                    if (i > 0)
                    {
                        s = s + ", ";
                    }
                    std::string localVarNameItem = dataFieldDictionaryDisplay(WDF.params[idx].get()->returnNameItems[i]);
                    if (decompOptVariableDeclare)
                    {
                        bool varNotDeclared = true;
                        for (int ii = 0; ii < localVarNamesT.size(); ii++)
                        {
                            if (localVarNameItem == localVarNamesT[ii])
                            {
                                varNotDeclared = false;
                            }
                        }
                        if (varNotDeclared)
                        {
                            localVarNamesD.push_back(WDF.params[idx].get()->depth);
                            localVarNamesT.push_back(localVarNameItem);
                            s = s + valueTypeName(dataFieldDictionary[dataFieldDictionaryIdx(WDF.params[idx].get()->returnNameItems[i])].fieldType) + " ";
                        }
                    }
                    s = s + localVarNameItem;
                }
                s = s + "} = ";
            }
            else
            {
                std::string localVarNameItem = dataFieldDictionaryDisplay(WDF.params[idx].get()->returnName);
                if (decompOptVariableDeclare)
                {
                    bool varNotDeclared = true;
                    for (int ii = 0; ii < localVarNamesT.size(); ii++)
                    {
                        if (localVarNameItem == localVarNamesT[ii])
                        {
                            varNotDeclared = false;
                        }
                    }
                    if (varNotDeclared)
                    {
                        localVarNamesD.push_back(WDF.params[idx].get()->depth);
                        localVarNamesT.push_back(localVarNameItem);
                        s = s + valueTypeName(dataFieldDictionary[dataFieldDictionaryIdx(WDF.params[idx].get()->returnName)].fieldType) + " ";
                    }
                }
                s = s + localVarNameItem + " = ";
            }
        }

        // Get instruction text
        ss = WDF.params[idx].get()->instrText();
        bool isBlockBegin = instrBlockBegin(ss);
        bool isBlockEnd = instrBlockEnd(ss);

        // Substitute constants and variables
        int fieldI1 = ss.find("fld_");
        int fieldI2 = ss.find("_fld");
        while ((fieldI1 >= 0) && (fieldI2 > fieldI1))
        {
            std::string s_pre = ss.substr(0, fieldI1);
            std::string s_value = dataFieldDictionaryDisplay(ss.substr(fieldI1, fieldI2 - fieldI1 + 4));
            std::string s_suf = ss.substr(fieldI2 + 4);
            if ((!s_value.empty()) && ((s_value[0] == '-') || (s_value[0] == '+')))
            {
                bool negBracket = true;
                if ((s_pre.size() >= 1) && (s_pre[s_pre.size() - 1] == '('))
                {
                    negBracket = false;
                }
                if ((s_pre.size() >= 1) && (s_pre[s_pre.size() - 1] == ','))
                {
                    negBracket = false;
                }
                if ((s_pre.size() >= 2) && (s_pre[s_pre.size() - 1] == ' ') && (s_pre[s_pre.size() - 2] == ','))
                {
                    negBracket = false;
                }
                if (negBracket)
                {
                    s_value = "(" + s_value + ")";
                }
            }
            ss = s_pre + s_value + s_suf;
            fieldI1 = ss.find("fld_");
            fieldI2 = ss.find("_fld");
        }

        int ____ = 1;
        if (codeInstrInfoStack)
        {
            while (((int)ss.find("[\\n]")) >= 0)
            {
                ss = hex::StringFindReplaceFirst(ss, "[\\n]", "[\\*][[^" + std::to_string(____) + "^]]" + hex::indent(WDF.params[idx].get()->depth));
                ____++;
            }
        }
        else
        {
            ss = hex::StringFindReplace(ss, "[\\n]", "[\\*]" + hex::indent(WDF.params[idx].get()->depth));
        }
        ss = hex::StringFindReplace(ss, "[\\*]", "[\\n]");

        // Append instruction text
        s = s + ss;

        if (codeInstrInfoStack)
        {
            std::string stackInfo = "";
            if (debugInfo)
            {
                stackInfo = stackInfo + std::to_string(WDF.params[idx].get()->id);
                if (!WDF.params[idx].get()->isFoldable)
                {
                    stackInfo = stackInfo + "*";
                }
                if (WDF.params[idx].get()->blockFold)
                {
                    stackInfo = stackInfo + "#";
                }
                if (WDF.params[idx].get()->isFoldable && (!WDF.params[idx].get()->blockFold))
                {
                    stackInfo = stackInfo + "~";
                }
            }
            if (((!decompOptFold) || (decompOptStackSimplify)) && (WDF.params[idx].get()->stackPrint))
            {
                stackInfo = stackInfo + stackPrintInfoFull(WDF.params[idx].get()->stackS__, WDF.params[idx].get()->stackI__, WDF.params[idx].get()->stackP__, WDF.params[idx].get()->stackO__, WDF.params[idx].get()->stackR__);
            }
            else
            {
                stackInfo = stackInfo + stackPrintInfoBlank();
            }

            int t = 0;
            if (isBlockBegin && isBlockEnd)
            {
                t = ____ - 2;
                if (t < 0) t = 0;
            }
            else
            {
                if (isBlockBegin)
                {
                    t = ____ - 1;
                    if (t < 0) t = 0;
                }
            }

            while (____ >= 0)
            {
                if (____ == t)
                {
                    s = hex::StringFindReplace(s, "[[^" + std::to_string(____) + "^]]", stackInfo);
                }
                else
                {
                    s = hex::StringFindReplace(s, "[[^" + std::to_string(____) + "^]]", hex::pad(stackInfo.size()));
                }
                ____--;
            }
        }

        if (WDF.params[idx].get()->printComma)
        {
            if (s[s.size() - 1] == ' ')
            {
                s = s.substr(0, s.size() - 1);
            }
            s = s + ";";
        }
        return s;
    }

    if (debugPrintVariableList && (idx == WDF.params.size()))
    {
        return debugValues();
    }

    return "`";
}

std::string wasmDecompiler::htmlPrefix(int id, int idx)
{
    if (useHtml)
    {
        if (id >= 100)
        {
            return hex::strToHtmlRaw() + "<a href=\"javascript:void(0)\" onclick=\"return sectionItem(" + std::to_string(id) + ", " + std::to_string(idx) + ")\">";
        }
        else
        {
            return hex::strToHtmlRaw() + "<span id=\"section" + std::to_string(id) + "item" + std::to_string(idx) + "\">" + hex::strToHtmlRaw();
        }
    }
    return "";
}

std::string wasmDecompiler::htmlSuffix(int id, int idx)
{
    if (useHtml)
    {
        if (id >= 100)
        {
            return "</a>" + hex::strToHtmlRaw();
        }
        else
        {
            return hex::strToHtmlRaw() + "</span>" + hex::strToHtmlRaw();
        }
    }
    return "";
}
