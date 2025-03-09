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
        case 0x01: return "u";     // !!!!
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
    stackSizeBlock.clear();
    stackSizeBlock.push_back(0);
    stackSizeIf.clear();
    stackSizeIf.push_back(-1);
    stackSizeDiff.clear();
    stackSizeDiff.push_back(0);
    tempVarCounterP = 1;
    tempVarCounterR = 1;
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
                        return typePrefix + "param" + std::to_string(dataFieldDictionary[i].fieldNumber);
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
                        if (multiType)
                        {
                            return typePrefix + dataFieldDictionary[i].fieldCategory + std::to_string(dataFieldDictionary[i].fieldNumber) + "_" + valueTypeName(dataFieldDictionary[i].fieldType & 255);
                        }
                        else
                        {
                            return typePrefix + dataFieldDictionary[i].fieldCategory + std::to_string(dataFieldDictionary[i].fieldNumber);
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
    if (category == "stack")
    {
        //_.fieldId = "fld_" + std::to_string(dataFieldDictionary.size()) + "_fld";
        _.fieldId = "fld_" + category + "_" + std::to_string(num) + "_" + std::to_string(dataFieldDictionary.size()) + "_fld";
    }
    if ((category != "const") && (category != "stack"))
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

void wasmDecompiler::convertBlockToLabels()
{
    if (decompBranch == 0)
    {
        return;
    }

    std::vector<int> flattenBegin;
    std::vector<int> flattenEnd;

    bool labelEndUsed = false;

    // Find branch and insert labels
    int labelCounter = 1;
    for (int i = 0; i < WDF.params.size(); i++)
    {
        if (WDF.params[i].get()->branchDepth > 0)
        {
            std::vector<int> depthToFind;
            std::vector<std::string> branchLabel;
            depthToFind.clear();
            branchLabel.clear();

            if (WDF.params[i].get()->branchDepth == branchDepthMagicNum)
            {
                std::string tempSeq = WDF.params[i].get()->params[0].get()->name + ",";
                int idx = 0;
                while (idx < (tempSeq.length() - 1))
                {
                    std::string tempVal = "";
                    while ((tempSeq[idx] < '0') || (tempSeq[idx] > '9'))
                    {
                        idx++;
                    }
                    while ((tempSeq[idx] >= '0') && (tempSeq[idx] <= '9'))
                    {
                        tempVal.push_back(tempSeq[idx]);
                        idx++;
                    }
                    while ((tempSeq[idx] != ','))
                    {
                        idx++;
                    }
                    int tempValI = atoi(tempVal.c_str()) + 1;
                    depthToFind.push_back(WDF.params[i].get()->depth - tempValI);
                }
            }
            else
            {
                depthToFind.push_back(WDF.params[i].get()->depth - WDF.params[i].get()->branchDepth);
            }

            for (int depthToFindI = 0; depthToFindI < depthToFind.size(); depthToFindI++)
            {
                int idTemp;

                int i_begin = i;
                int i_end = i;
                int i_begin0 = i;
                int i_end0 = i;

                std::string labelName = "__label_end__";
                if (depthToFind[depthToFindI] > 0)
                {
                    // Find block begin
                    while (WDF.params[i_begin].get()->depth > depthToFind[depthToFindI])
                    {
                        i_begin--;
                    }
                    i_begin0 = i_begin;
                    idTemp = WDF.params[i_begin0].get()->id;
                    while (WDF.params[i_begin0].get()->id == idTemp)
                    {
                        i_begin0--;
                    }

                    // Find block end
                    while (WDF.params[i_end].get()->depth > depthToFind[depthToFindI])
                    {
                        i_end++;
                    }
                    i_end0 = i_end;
                    idTemp = WDF.params[i_end0].get()->id;
                    while (WDF.params[i_end0].get()->id == idTemp)
                    {
                        i_end0++;
                    }

                    // Create label name
                    switch (WDF.params[i_begin].get()->branchType)
                    {
                        case 1:
                        case 2:
                            labelName = "__label_" + std::to_string(labelCounter) + "_below__";
                            break;
                        case 3:
                            labelName = "__label_" + std::to_string(labelCounter) + "_above__";
                            break;
                    }

                    // Insert label instruction
                    branchLabel.push_back(labelName);
                    WDF.additionalInstr(0, "", labelName + ":", 0);
                    std::shared_ptr<wasmDecompilerFunction> Instr = WDF.params[WDF.params.size() - 1];
                    WDF.params.pop_back();
                    Instr.get()->id = WDF.params.size() + 1;
                    Instr.get()->depth = depthToFind[depthToFindI];
                    Instr.get()->branchType = 9;

                    switch (WDF.params[i_begin].get()->branchType)
                    {
                        case 1:
                        case 2:
                            {
                                int i_end0_i = i_end0;
                                while (WDF.params[i_end0_i].get()->branchType == 9)
                                {
                                    i_end0_i++;
                                }
                                WDF.params.insert(WDF.params.begin() + i_end0_i, Instr);
                            }
                            break;
                        case 3:
                            {
                                WDF.params.insert(WDF.params.begin() + i_begin0 + 1, Instr);
                                i_begin0++;
                                i_begin++;
                                i_end++;
                                i_end0++;
                                i++;
                            }
                            break;
                    }


                    // Add to flatten code list
                    if (WDF.params[i_begin].get()->branchType != 2)
                    {
                        flattenBegin.push_back(WDF.params[i_begin].get()->id);
                        flattenEnd.push_back(WDF.params[i_end].get()->id);
                    }

                    labelCounter++;
                }
                else
                {
                    branchLabel.push_back(labelName);

                    labelEndUsed = true;
                }
            }

            // Replace the depth with labels
            if (WDF.params[i].get()->branchDepth == branchDepthMagicNum)
            {
                std::string labelSet = "";
                for (int branchLabelI = 0; branchLabelI < branchLabel.size(); branchLabelI++)
                {
                    if (branchLabelI > 0)
                    {
                        labelSet = labelSet + ", ";
                    }
                    labelSet = labelSet + branchLabel[branchLabelI];
                }
                WDF.params[i].get()->paramAdd(labelSet);
                WDF.params[i].get()->params[0].get()->name = labelSet;
            }
            else
            {
                WDF.params[i].get()->params.pop_back();
                WDF.params[i].get()->paramAdd(branchLabel[0]);
            }
        }
    }

    // Insert end label instruction
    if (labelEndUsed)
    {
        std::string labelName = "__label_end__";
        WDF.additionalInstr(0, "", labelName + ":", 0);
        std::shared_ptr<wasmDecompilerFunction> Instr = WDF.params[WDF.params.size() - 1];
        WDF.params.pop_back();
        Instr.get()->id = WDF.params.size() + 1;
        Instr.get()->depth = 1;
        Instr.get()->branchType = 9;
        WDF.params.insert(WDF.params.end() - 1, Instr);
    }

    // Flatten code block
    if (decompBranch == 2)
    {
        std::vector<int> fllatenDoneB;
        std::vector<int> fllatenDoneE;
        for (int i = 0; i < flattenBegin.size(); i++)
        {
            bool doFlat = true;
            for (int ii = 0; ii < fllatenDoneB.size(); ii++)
            {
                if (fllatenDoneB[ii] == flattenBegin[i])
                {
                    doFlat = false;
                }
                if (fllatenDoneE[ii] == flattenEnd[i])
                {
                    doFlat = false;
                }
            }

            if (doFlat)
            {
                int idx1 = WDF.params.size() - 1;
                while (WDF.params[idx1].get()->id != flattenBegin[i])
                {
                    idx1--;
                }
                int idx2 = 0;
                while (WDF.params[idx2].get()->id != flattenEnd[i])
                {
                    idx2++;
                }
                for (int ii = idx1 + 1; ii < idx2; ii++)
                {
                    WDF.params[ii].get()->depth--;
                }
                fllatenDoneB.push_back(flattenBegin[i]);
                fllatenDoneE.push_back(flattenEnd[i]);

                WDF.params.erase(WDF.params.begin() + idx2);
                WDF.params.erase(WDF.params.begin() + idx1);
            }
        }
    }

    // Optimize labels
    if (!debugNoLabelOptimize)
    {
        for (int i = 0; i < WDF.params.size(); i++)
        {
            if (WDF.params[i].get()->branchType == 9)
            {
                std::string labelGroupName = "";
                std::string labelName;

                // Find the group range and create group name
                int i1 = i;
                while (WDF.params[i].get()->branchType == 9)
                {
                    labelName = WDF.params[i].get()->name;
                    labelName = labelName.substr(0, labelName.size() - 1);
                    if (hex::StringIndexOf(labelName, "_end_") >= 0)
                    {
                        labelGroupName = labelName;
                    }
                    i++;
                }
                i--;
                int i2 = i;
                labelName = WDF.params[i1].get()->name;
                if (labelGroupName == "") labelGroupName = labelName.substr(0, labelName.size() - 1);

                // Replace the label names in branch instructions
                for (int ii = i1; ii <= i2; ii++)
                {
                    labelName = WDF.params[ii].get()->name;
                    labelName = labelName.substr(0, labelName.size() - 1);
                    if (labelName != labelGroupName)
                    {
                        for (int iii = 0; iii < WDF.params.size(); iii++)
                        {
                            if (WDF.params[iii].get()->branchDepth > 0)
                            {
                                int parN = WDF.params[iii].get()->params.size() - 1;
                                if (WDF.params[iii].get()->branchDepth != branchDepthMagicNum)
                                {
                                    WDF.params[iii].get()->params[parN].get()->name = labelGroupName;
                                }
                                else
                                {
                                    std::string tempList = " " + WDF.params[iii].get()->params[0].get()->name + ",";
                                    tempList = hex::StringFindReplace(tempList, " " + labelName + ",", " " + labelGroupName + ",");
                                    WDF.params[iii].get()->params[0].get()->name = tempList.substr(1, tempList.size() - 2);
                                }
                            }
                        }
                    }
                }

                // Remove redundant labels
                for (int ii = i1; ii <= i2; ii++)
                {
                    labelName = WDF.params[ii].get()->name;
                    labelName = labelName.substr(0, labelName.size() - 1);
                    if (labelName != labelGroupName)
                    {
                        WDF.params.erase(WDF.params.begin() + ii);
                        ii--;
                        i2--;
                        i--;
                    }
                }

            }
        }
    }
}

void wasmDecompiler::codeOptimize()
{
    // Prepare the result type from function type or first parameter type
    for (int i = 0; i < WDF.params.size(); i++)
    {
        if (WDF.params[i].get()->returnName != "")
        {
            if (dataFieldDictionaryGetType(WDF.params[i].get()->returnName, -1) == 0)
            {
                switch (WDF.params[i].get()->returnType)
                {
                    case 0:
                        break;
                    case fieldTypeCallFunc:
                        for (int ii = 0; ii < WDF.params[i].get()->returnTypeList.size(); ii++)
                        {
                            std::string returnVarName = hex::StringGetParam(WDF.params[i].get()->returnName.substr(0, WDF.params[i].get()->returnName.size() - 1) + ",", ii, ',').substr(1);
                            dataFieldDictionarySetType(returnVarName, WDF.params[i].get()->returnTypeList[ii]);
                        }
                        break;
                    case fieldTypeVariable:
                        if (WDF.params[i].get()->params.size() > 0)
                        {
                            dataFieldDictionarySetType(WDF.params[i].get()->returnName, dataFieldDictionaryGetType(WDF.params[i].get()->params[0].get()->name, -1) & 255);
                        }
                        break;
                    default:
                        dataFieldDictionarySetType(WDF.params[i].get()->returnName, WDF.params[i].get()->returnType);
                        break;
                }
            }
        }
    }


    int work = decompOptFold ? 1000000000 : 0;
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


        s = hex::indent(WDF.params[idx].get()->depth);
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

        // Substitute constants and variables
        int fieldI1 = ss.find("fld_");
        int fieldI2 = ss.find("_fld");
        while ((fieldI1 >= 0) && (fieldI2 > fieldI1))
        {
            std::string s_pre = ss.substr(0, fieldI1);
            std::string s_value = dataFieldDictionaryDisplay(ss.substr(fieldI1, fieldI2 - fieldI1 + 4));
            std::string s_suf = ss.substr(fieldI2 + 4);
            if ((s_value.size() > 0) && ((s_value[0] == '-') || (s_value[0] == '+')))
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

        // Append instruction text
        s = s + ss;


        s = hex::StringFindReplace(s, "[\\n]", "[\\*]" + hex::indent(WDF.params[idx].get()->depth));
        s = hex::StringFindReplace(s, "[\\*]", "[\\n]");
        if (debugInfo)
        {
            std::string debugPrefix = std::to_string(idx) + ". " + std::to_string(WDF.params[idx].get()->id) + (WDF.params[idx].get()->isFoldable ? " " : "*") + (WDF.params[idx].get()->blockFold ? "#" : " ") + "   ";
            if (!decompOptFold)
            {
                if (WDF.params[idx].get()->stackPrint)
                {
                    debugPrefix = debugPrefix + std::to_string(WDF.params[idx].get()->stackI) + " " + std::to_string(WDF.params[idx].get()->stackP) + " ";
                    debugPrefix = debugPrefix + std::to_string(WDF.params[idx].get()->stackR) + " " + std::to_string(WDF.params[idx].get()->stackO);
                }
            }
            while (debugPrefix.size() < 22)
            {
                debugPrefix = debugPrefix + " ";
            }
            if (WDF.params[idx].get()->depth < 10)
            {
                debugPrefix = debugPrefix + " ";
            }
            debugPrefix = debugPrefix + std::to_string(WDF.params[idx].get()->depth) + "  ";


            s = debugPrefix + hex::StringFindReplace(s, "[\\n]", "[\\*]" + debugPrefix);
            s = hex::StringFindReplace(s, "[\\*]", "[\\n]");
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
