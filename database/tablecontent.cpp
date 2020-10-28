//
// Created by Chestnut on 2020/11/11.
//

#include "database.h"

#include "tablecontent.h"

#include "table.h"

#include "record.h"

fs::path TableContent::getFilePath()
{
    return _table._database._database_path / (fmt::format("{}.db", _table._name));
}

TableContent::TableContent(Table &table) : _table(table)
{
}

void TableContent::del()
{
    deleteFile();
}

void TableContent::create()
{
    if (file_exist())
        throw std::invalid_argument(fmt::format("Table {} already exists!", _table._name));

    blocks_init();
}

void TableContent::insert(const std::vector<std::tuple<std::string, std::optional<std::string>>> &values)
{
    Record _record(_table);
    for (auto &[col_name, value] : values)
    {
        if (value.has_value())
        {
            auto field = _record.entry(col_name);
            field->from_string(value.value());
        }
    }

    for (auto &[col_id, name] : _table._metadata.col_names)
    {
        auto &col = _table._metadata.cols.at(col_id);
        if (col.default_value.has_value() && !_record.entry(name)->not_null())
        {
            _record.entry(name)->from_string(col.default_value.value());
        }
    }
    _record.validation();
    auto bytes = _record.to_bytes();
    file_blocks<TableContent>::insert(begin(), bytes.get());
}

int TableContent::getBlockSize()
{
    int result = 0;
    for (auto &[col_id, name] : _table._metadata.col_names)
    {
        auto &col = _table._metadata.cols.at(col_id);
        auto &&field = col.getField();
        result += field->size();
    }
    return result;
}

TableContent::range_iterator TableContent::range_begin(std::shared_ptr<RecordChecker> checker)
{
    return ++TableContent::range_iterator(*this, 0, _table, checker);
}

TableContent::range_iterator TableContent::end()
{
    return TableContent::range_iterator(*this, 0, _table, nullptr);
}

TableContent::range_iterator::_Self &TableContent::range_iterator::operator++()
{
    Record _record(_table);
    do
    {
        _node = get_block()->next;
        _block = nullptr;
        if (!_node)
            return *this;
        auto data = getData();
        _record.from_data(data.get());
    } while (!_checker->check(_record));
    return *this;
}

TableContent::range_iterator::_Self &TableContent::range_iterator::operator--()
{
    Record _record(_table);
    do
    {
        _node = get_block()->prev;
        _block = nullptr;
        if (!_node)
            return *this;
        auto data = getData();
        _record.from_data(data.get());
    } while (!_checker->check(_record));
    return *this;
}