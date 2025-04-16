#include "wasmdecompiler.h"

void wasmDecompiler::convertBlockToLabels()
{
    if (decompBranch == 0)
    {
        return;
    }

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

                std::string labelName = "label_end";
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
                            labelName = "label_" + std::to_string(labelCounter) + "_below";
                            break;
                        case 3:
                            labelName = "label_" + std::to_string(labelCounter) + "_above";
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
                        //!!flattenBegin.push_back(WDF.params[i_begin].get()->id);
                        //!!flattenEnd.push_back(WDF.params[i_end].get()->id);
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
        std::string labelName = "label_end";
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
        std::vector<int> flattenBegin;
        std::vector<int> flattenEnd;
        for (int i = 0; i < WDF.params.size(); i++)
        {
            if ((WDF.params[i].get()->branchType == branchBlock) || (WDF.params[i].get()->branchType == branchLoop))
            {
                for (int ii = i; ii < WDF.params.size(); ii++)
                {
                    if ((WDF.params[ii].get()->branchType == branchEnd) && (WDF.params[ii].get()->depth == WDF.params[i].get()->depth))
                    {
                        flattenBegin.push_back(WDF.params[i].get()->id);
                        flattenEnd.push_back(WDF.params[ii].get()->id);
                        break;
                    }
                }
            }
        }

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
        std::string labelGroupName = "";
        std::string labelGroupNameX = "";
        std::string labelName = "";
        std::string labelNameX = "";
        int WDF_params_size = WDF.params.size();
        for (int i = 0; i < WDF_params_size; i++)
        {
            if (WDF.params[i].get()->branchType == 9)
            {
                if (!labelGroupName.empty())
                {
                    labelName = WDF.params[i].get()->name;
                    labelNameX = labelName.substr(0, labelName.size() - 1);

                    // Replace the label names in branch instructions
                    for (int ii = 0; ii < WDF_params_size; ii++)
                    {
                        if (WDF.params[ii].get()->branchDepth > 0)
                        {
                            int parN = WDF.params[ii].get()->params.size() - 1;
                            if (WDF.params[ii].get()->branchDepth != branchDepthMagicNum)
                            {
                                if (WDF.params[ii].get()->params[parN].get()->name == labelNameX)
                                {
                                    WDF.params[ii].get()->params[parN].get()->name = labelGroupNameX;
                                }
                            }
                            else
                            {
                                std::string tempList = " " + WDF.params[ii].get()->params[0].get()->name + ",";
                                tempList = hex::StringFindReplace(tempList, " " + labelNameX + ",", " " + labelGroupNameX + ",");
                                WDF.params[ii].get()->params[0].get()->name = tempList.substr(1, tempList.size() - 2);
                            }
                        }
                    }

                    // Remove redundant label
                    WDF.params.erase(WDF.params.begin() + i);
                    i--;
                    WDF_params_size--;
                }
                else
                {
                    labelGroupName = WDF.params[i].get()->name;
                    labelGroupNameX = labelGroupName.substr(0, labelGroupName.size() - 1);
                }
            }
            else
            {
                labelGroupName = "";
            }
        }
    }
}

void wasmDecompiler::codeOptimize()
{
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
                            // Function can not be folded, when the function result is used more than once
                            assign = WDF.params.size() + 1;
                        }
                    }
                }

                // The first unfoldaable function
                if (WDF.params[i].get()->blockFold)
                {
                    spaceBorder = i;
                    break;
                }

                // Second result assignment
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
}
