//
// Created by Chestnut on 2020/11/10.
//

#include "tablemetadata.h"
#include "database.h"
#include "field_implements/field_implements.h"
#include "table.h"

void TableMetadata::create(const std::vector<std::tuple<std::string, Column>> &_cols)
{
    if (exist())
        throw std::invalid_argument(fmt::format("Table {} already exists!", _table._name));

    decltype(col_names) new_col_names;
    decltype(cols) new_cols;

    int i = 0;
    for (auto [name, col] : _cols)
    {
        if (new_col_names.right.count(name))
            throw std::invalid_argument(fmt::format("Column {} re-defined!", name));
        new_col_names.left.insert(std::make_pair(i, name));
        new_cols.emplace_back(col);
        i++;
    }
    col_names = std::move(new_col_names);
    cols = std::move(new_cols);

    save();
}

bool TableMetadata::exist()
{
    return archive_exist();
}

TableMetadata::TableMetadata(const Table &table) : _table(table)
{
}

fs::path TableMetadata::getFilePath()
{
    return _table._database._database_path / (fmt::format("{}.frm", _table._name));
}

void TableMetadata::init()
{
    if (exist())
        read();
}

void TableMetadata::del()
{
    if (!exist())
        throw std::invalid_argument(fmt::format("Table {} doesn't exists!", _table._name));
    archive_del();
}