#include "netvars_manager.hpp"
#include <cctype>
#include <cstring>

int netvars::get_offset(const char *tableName, const char *propName) 
{
    if (!tableName || !propName)
        return 0;
    int offs = get_prop(tableName, propName);
    return offs;
}

int netvars::get_prop(const char *tableName, const char *propName, RecvProp **prop) 
{
    if (!tableName || !propName)
        return 0;

    RecvTable *recvTable = this->get_table(tableName);
    if (!recvTable)
        return 0;

    int offs = get_prop(recvTable, propName, prop);
    return offs;
}

int netvars::get_prop(RecvTable *recvTable, const char *propName, RecvProp **prop) 
{
    if (!recvTable || !propName)
        return 0;

    for (int i = 0; i < recvTable->propCount; i++) 
    {
        auto *recvProp = &recvTable->props[i];
        if (!recvProp)
            continue;

        auto recvChild = recvProp->dataTable;

        if (recvChild && recvChild->propCount > 0) 
        {
            int tmp = get_prop(recvChild, propName, prop);
            if (tmp)
                return recvProp->offset + tmp;
        }

        if (!recvProp->name)
            continue;

        if (strcmp(recvProp->name, propName) != 0)
            continue;

        if (prop)
            *prop = recvProp;

        return recvProp->offset;
    }

    return 0;
}

RecvTable *netvars::get_table(const char *tableName) 
{
    if (tables.empty() || !tableName)
        return nullptr;

    for (auto &table : tables) 
    {
        if (table.first == tableName)
            return table.second;
    }

    return nullptr;
}
