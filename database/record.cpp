//
// Created by Chestnut on 2020/11/10.
//

#include "record.h"
#include "database.h"

Record::Record(Table &table) : _table(table)
{
    int _offset_counter = 0;

    for (auto &[col_id, name] : _table._metadata.col_names)
    {
        auto &col = _table._metadata.cols.at(col_id);
        entries[name] = col.getField();
        _bytes_offset[name] = _offset_counter;
        _offset_counter += entries[name]->size();
    }
}

std::shared_ptr<char> Record::to_bytes()
{
    auto bytes = std::shared_ptr<char>((char *)malloc(getBlockSize()));
    for (auto [name, offset] : _bytes_offset)
    {
        auto bytes_of_entry = entries.at(name)->to_bytes();
        memcpy(bytes.get() + offset, bytes_of_entry.get(), entries[name]->size());
    }
    return bytes;
}

BaseField *Record::entry(const std::string &name)
{
    if (!entries.count(name))
        throw std::domain_error(
            fmt::format("Column {} of {}.{} doesn't exists!", name, _table._database.name(), _table._name));
    return entries.at(name).get();
}

void Record::validation()
{
    for (auto &[name, field] : entries)
    {
        auto col_id = _table._metadata.col_names.right.at(name);
        auto &&col = _table._metadata.cols.at(col_id);
        if (col.not_null && !field->not_null())
        {
            throw std::domain_error(fmt::format("Entry {} of {}.{} is not allowed to be null!", name,
                                                _table._database.name(), _table._name));
        }
        if (field->not_null() && col.unique)
        {
            std::shared_ptr<RecordChecker> checker =
                std::make_shared<SimpleRecordChecker>(name, field->to_string().value());
            auto it = _table._content.range_begin(checker);
            if (it != _table._content.end())
            {
                throw std::domain_error(
                    fmt::format("Entry {} of {}.{} was set UNIQUE but this value {} already exists!", name,
                                _table._database.name(), _table._name, field->to_string().value()));
            }
        }
        if (col._length.has_value())
        {
        }
    }
}

void Record::from_data(char *data)
{
    for (auto [name, offset] : _bytes_offset)
    {
        auto bytes_of_entry = entries.at(name)->size();
        auto bytes = std::unique_ptr<char>((char *)malloc(bytes_of_entry));
        memcpy(bytes.get(), data + offset, bytes_of_entry);
        entries.at(name)->from_bytes(bytes.get());
    }
}

int Record::getBlockSize()
{
    return _table._content.getBlockSize();
}
