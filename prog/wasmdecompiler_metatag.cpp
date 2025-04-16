#include "wasmdecompiler.h"

void wasmDecompiler::metaTagClear()
{
    metaTagValid = true;
    metaTag.clear();
    metaTagCache.clear();
}

void wasmDecompiler::metaTagAdd(int section, int type, int idx, int idxx, std::string name)
{
    if (name.empty())
    {
        return;
    }

    metaTagDef _;
    _.section = section;
    _.type = type;
    _.idx = idx;
    _.idxx = idxx;
    _.name = name;
    _.sysname = correctFunctionName(name);

    // Locals
    // Globals
    // Functions
    int type_ = type % 100;
    if ((type_ == 2) || (type_ == 7) || (type_ == 1))
    {
        // Add distinguishing "_" when name starts from "reserved" name
        bool isSpecialName = false;
        int sysnameSize = _.sysname.size();
        if ((sysnameSize >= 8) && (_.sysname.substr(0, 8) == "function")) isSpecialName = true;
        if ((sysnameSize >= 6) && (_.sysname.substr(0, 6) == "global")) isSpecialName = true;
        if ((sysnameSize >= 5) && (_.sysname.substr(0, 5) == "label")) isSpecialName = true;
        if ((sysnameSize >= 5) && (_.sysname.substr(0, 5) == "local")) isSpecialName = true;
        if ((sysnameSize >= 5) && (_.sysname.substr(0, 5) == "stack")) isSpecialName = true;
        if ((sysnameSize >= 5) && (_.sysname.substr(0, 5) == "tempp")) isSpecialName = true;
        if ((sysnameSize >= 5) && (_.sysname.substr(0, 5) == "tempr")) isSpecialName = true;
        if ((sysnameSize >= 5) && (_.sysname.substr(0, 5) == "param")) isSpecialName = true;

        if (isSpecialName)
        {
            if (_.sysname[sysnameSize - 1] != '_')
            {
                _.sysname = _.sysname + "_";
            }
        }
    }

    metaTag.push_back(_);
}

void wasmDecompiler::metaTagRemoveLast(int n)
{
    metaTagValid = false;
    while (n > 0)
    {
        metaTag.pop_back();
        n--;
    }
}

std::string wasmDecompiler::metaTagGetInfo(int section, int vecIdx)
{
    if (vecIdx < metaTag.size())
    {
        metaTagDef _ = metaTag[vecIdx];
        std::string info = "unknown metatag " + std::to_string(_.type);
        switch (_.type)
        {
            case 0: info = "module \"" + _.name + "\""; break;
            case 1: info = "function[" + std::to_string(_.idx) + "] \"" + _.name + "\""; break;
            case 2: info = "function[" + std::to_string(_.idx) + "].local[" + std::to_string(_.idxx) + "] \"" + _.name + "\""; break;
            //case 3: info = "label[" + std::to_string(_.idx) + "] \"" + _.name + "\""; break;
            case 4: info = "type[" + std::to_string(_.idx) + "] \"" + _.name + "\""; break;
            case 5: info = "table[" + std::to_string(_.idx) + "] \"" + _.name + "\""; break;
            case 6: info = "memory[" + std::to_string(_.idx) + "] \"" + _.name + "\""; break;
            case 7: info = "global[" + std::to_string(_.idx) + "] \"" + _.name + "\""; break;
            case 8: info = "elem[" + std::to_string(_.idx) + "] \"" + _.name + "\""; break;
            case 9: info = "data[" + std::to_string(_.idx) + "] \"" + _.name + "\""; break;
            case 11: info = "tag[" + std::to_string(_.idx) + "] \"" + _.name + "\""; break;

            case 101: info = "import_function[" + std::to_string(_.idx) + "] \"" + _.name + "\""; break;
            case 201: info = "export_function[" + std::to_string(_.idx) + "] \"" + _.name + "\""; break;

            case 107: info = "import_global[" + std::to_string(_.idx) + "] \"" + _.name + "\""; break;
            case 207: info = "export_global[" + std::to_string(_.idx) + "] \"" + _.name + "\""; break;

            case 105: info = "import_table[" + std::to_string(_.idx) + "] \"" + _.name + "\""; break;
            case 205: info = "export_table[" + std::to_string(_.idx) + "] \"" + _.name + "\""; break;

            case 106: info = "import_memory[" + std::to_string(_.idx) + "] \"" + _.name + "\""; break;
            case 206: info = "export_memory[" + std::to_string(_.idx) + "] \"" + _.name + "\""; break;
        }

        if (_.section != section)
        {
            return "#";
        }

        if (_.type >= 100)
        {
            return "#";
        }

        return info;
    }
    else
    {
        return "`";
    }
}

std::string wasmDecompiler::metaTagGetTempl(int type, int idx, std::string namePre, std::string nameSuf)
{
    std::string _ = metaTagGet(type, idx, "");
    if (!_.empty())
    {
        return namePre + _ + nameSuf;
    }
    else
    {
        return "";
    }
}

std::string wasmDecompiler::metaTagGet(int type, int idx, std::string name)
{
    std::string id = std::to_string(type) + "_" + std::to_string(idx) + "_" + name;
    if (metaTagCache.find(id) != metaTagCache.end())
    {
        return metaTagCache[id];
    }

    if (useTags)
    {
        std::string name_tag = "";
        std::string name_imp = "";
        std::string name_exp = "";
        std::string name_tmp;

        bool printQuote = false;
        bool printSystem = false;
        int type_ = type % 100;
        if ((type >= 200) && (type < 400))
        {
            printQuote = true;
            type -= 200;
        }
        if (type >= 100)
        {
            printSystem = true;
        }
        for (int i = 0; i < metaTag.size(); i++)
        {
            if (((metaTag[i].type % 100) == type_) && (metaTag[i].idx == idx))
            {
                if (printSystem)
                {
                    name_tmp = metaTag[i].sysname;
                }
                else
                {
                    name_tmp = metaTag[i].name;
                }
                switch (metaTag[i].type / 100)
                {
                    case 0: name_tag = name_tmp; break;
                    case 1: name_imp = name_tmp; break;
                    case 2: name_exp = name_tmp; break;
                }
            }
        }

        if (printQuote)
        {
            name_tmp = "";
            if ((!name_tag.empty()))
            {
                if (!name_tmp.empty()) name_tmp = name_tmp + ",";
                name_tmp = name_tmp + "\"" + name_tag + "\"";
            }
            if ((!name_exp.empty()) && (name_exp != name_tag))
            {
                if (!name_tmp.empty()) name_tmp = name_tmp + ",";
                name_tmp = name_tmp + "\"" + name_exp + "\"";
            }
            if ((!name_imp.empty()) && (name_imp != name_tag) && (name_imp != name_exp))
            {
                if (!name_tmp.empty()) name_tmp = name_tmp + ",";
                name_tmp = name_tmp + "\"" + name_imp + "\"";
            }
            metaTagCache[id] = name_tmp;
            return name_tmp;
        }
        else
        {
            if (!name_tag.empty())
            {
                metaTagCache[id] = name_tag;
                return name_tag;
            }
            if (!name_exp.empty())
            {
                metaTagCache[id] = name_exp;
                return name_exp;
            }
            if (!name_imp.empty())
            {
                metaTagCache[id] = name_imp;
                return name_imp;
            }
        }
    }
    metaTagCache[id] = name;
    return name;
}

std::string wasmDecompiler::metaTagGet2(int type, int idx, int idxx, std::string name)
{
    std::string id = std::to_string(type) + "_" + std::to_string(idx) + "_" + std::to_string(idxx) + "_" + name;
    if (metaTagCache.find(id) != metaTagCache.end())
    {
        return metaTagCache[id];
    }

    if (useTags)
    {
        std::string name_tag = "";
        int type_ = type % 100;
        for (int i = 0; i < metaTag.size(); i++)
        {
            if ((metaTag[i].type == type_) && (metaTag[i].idx == idx) && (metaTag[i].idxx == idxx))
            {
                if (type >= 100)
                {
                    name_tag = metaTag[i].sysname;
                }
                else
                {
                    name_tag = metaTag[i].name;
                }
            }
        }

        if (!name_tag.empty())
        {
            metaTagCache[id] = name_tag;
            return name_tag;
        }
    }
    metaTagCache[id] = name;
    return name;
}

void wasmDecompiler::metaTagValidateNames()
{
    int metaTagS = metaTag.size();

    // Distinguish repeating names
    bool repeatNames = true;
    while (repeatNames)
    {
        repeatNames = false;
        for (int x = 0; x < metaTagS; x++)
        {
            for (int y = 0; y < metaTagS; y++)
            {
                if ((metaTag[x].sysname == metaTag[y].sysname) && (x != y) && (metaTag[x].type == metaTag[y].type))
                {
                    metaTag[x].sysname = metaTag[x].sysname + "_1";
                    metaTag[y].sysname = metaTag[y].sysname + "_2";
                    repeatNames = true;
                }
            }
        }
    }

    // Clear cache
    metaTagCache.clear();
}

std::string wasmDecompiler::correctFunctionName(std::string funcName)
{
    std::string funcName0 = "";
    if (funcName.size() > 0)
    {
        if ((funcName[0] >= '0') && (funcName[0] <= '9'))
        {
            funcName0.push_back('_');
        }
    }
    for (int i = 0; i < funcName.size(); i++)
    {
        bool std = false;
        if ((funcName[i] >= '0') && (funcName[i] <= '9')) std = true;
        if ((funcName[i] >= 'A') && (funcName[i] <= 'Z')) std = true;
        if ((funcName[i] >= 'a') && (funcName[i] <= 'z')) std = true;
        funcName0.push_back(std ? funcName[i] : '_');
    }
    return funcName0;
}
