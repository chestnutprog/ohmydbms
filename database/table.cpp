//
// Created by Chestnut on 2020/11/10.
//

#include "database.h"

#include "table.h"

#include "record.h"

Table::Table(Database &database, const std::string &name)
    : _database{database}, _metadata(*this), _name(name), _content(*this)
{
    _metadata.init();
}

void Table::create(const std::vector<std::tuple<std::string, Column>> &cols)
{

    BOOST_LOG_TRIVIAL(info) << fmt::format("Create table {}.{}.", _database._name, _name);
    if (exist())
    {
        BOOST_LOG_TRIVIAL(warning) << fmt::format("Table {}.{} already exists!", _database._name, _name);
        throw std::invalid_argument(fmt::format("Table {}.{} already exists!", _database._name, _name));
    }
    _metadata.create(cols);
    _content.create();
    BOOST_LOG_TRIVIAL(info) << fmt::format("Table {}.{} created.", _database._name, _name);
}

bool Table::exist()
{
    return _metadata.exist();
}

void Table::del()
{
    BOOST_LOG_TRIVIAL(info) << fmt::format("Drop table {}.{}.", _database._name, _name);
    if (!exist())
    {
        BOOST_LOG_TRIVIAL(warning) << fmt::format("Table {}.{} doesn't exists!", _database._name, _name);
        throw std::invalid_argument(fmt::format("Table {}.{} doesn't exists!", _database._name, _name));
    }
    _content.del();
    _metadata.del();
    BOOST_LOG_TRIVIAL(info) << fmt::format("Table {}.{} dropped.", _database._name, _name);
}

DB_RESULT_TYPE Table::select()
{
    if (!exist())
    {
        BOOST_LOG_TRIVIAL(warning) << fmt::format("Table {}.{} doesn't exists!", _database._name, _name);
        throw std::invalid_argument(fmt::format("Table {}.{} doesn't exists!", _database._name, _name));
    }
    std::vector<std::string> col_names;
    std::vector<std::vector<std::optional<std::string>>> content;
    for (auto &[col_id, name] : _metadata.col_names)
    {
        auto &col = _metadata.cols.at(col_id);
        col_names.push_back(name);
    }

    for (auto it = _content.begin(); it != _content.end(); ++it)
    {
        std::vector<std::optional<std::string>> row;
        Record _record(*this);
        auto data = it.getData();
        _record.from_data(data.get());
        for (const auto &col_name : col_names)
        {
            auto entry = _record.entry(col_name);
            row.emplace_back(entry->to_string());
        }
        content.emplace_back(std::move(row));
    }

    return make_pair(std::move(col_names), std::move(content));
}

DB_RESULT_TYPE Table::select(std::shared_ptr<RecordChecker> checker)
{
    if (!exist())
    {
        BOOST_LOG_TRIVIAL(warning) << fmt::format("Table {}.{} doesn't exists!", _database._name, _name);
        throw std::invalid_argument(fmt::format("Table {}.{} doesn't exists!", _database._name, _name));
    }
    std::vector<std::string> col_names;
    std::vector<std::vector<std::optional<std::string>>> content;
    for (auto &[col_id, name] : _metadata.col_names)
    {
        auto &col = _metadata.cols.at(col_id);
        col_names.push_back(name);
    }

    for (auto it = _content.range_begin(checker); it != _content.end(); ++it)
    {
        std::vector<std::optional<std::string>> row;
        Record _record(*this);
        auto data = it.getData();
        _record.from_data(data.get());
        for (const auto &col_name : col_names)
        {
            auto entry = _record.entry(col_name);
            row.emplace_back(entry->to_string());
        }
        content.emplace_back(std::move(row));
    }

    return make_pair(std::move(col_names), std::move(content));
}

DB_RESULT_TYPE Table::desc()
{
    if (!exist())
    {
        BOOST_LOG_TRIVIAL(warning) << fmt::format("Table {}.{} doesn't exists!", _database._name, _name);
        throw std::invalid_argument(fmt::format("Table {}.{} doesn't exists!", _database._name, _name));
    }
    std::vector<std::string> col_names = {"col_name", "type", "length", "default", "primary", "not_null", "unique"};
    std::vector<std::vector<std::optional<std::string>>> content;

    for (const auto &[col_id, name] : _metadata.col_names)
    {
        auto &col = _metadata.cols.at(col_id);
        std::vector<std::optional<std::string>> row;
        row.emplace_back(name);
        row.emplace_back(BaseField::type_to_string(col.type));

        if (col._length.has_value())
            row.emplace_back(std::to_string(col._length.value()));
        else
            row.emplace_back();

        if (col.default_value.has_value())
            row.emplace_back(col.default_value.value());
        else
            row.emplace_back();

        row.emplace_back(BOOL_TO_STR(col.primary));
        row.emplace_back(BOOL_TO_STR(col.not_null));
        row.emplace_back(BOOL_TO_STR(col.unique));

        content.emplace_back(std::move(row));
    }

    return make_pair(std::move(col_names), std::move(content));
}

void Table::rename_col(const std::string &before, const std::string &after)
{
    if (_metadata.col_names.right.count(after))
    {
        throw std::invalid_argument(fmt::format("Column {} already exists!", after));
    }
    auto it = _metadata.col_names.right.find(before);
    _metadata.col_names.right.replace_key(it, after);

    _metadata.save();
}

DB_RESULT_TYPE Table::erase(std::shared_ptr<RecordChecker> checker)
{
    if (!exist())
    {
        BOOST_LOG_TRIVIAL(warning) << fmt::format("Table {}.{} doesn't exists!", _database._name, _name);
        throw std::invalid_argument(fmt::format("Table {}.{} doesn't exists!", _database._name, _name));
    }
    std::vector<std::string> col_names;
    std::vector<std::vector<std::optional<std::string>>> content;
    for (auto &[col_id, name] : _metadata.col_names)
    {
        auto &col = _metadata.cols.at(col_id);
        col_names.push_back(name);
    }

    for (auto it = _content.range_begin(checker); it != _content.end(); ++it)
    {
        std::vector<std::optional<std::string>> row;
        Record _record(*this);
        auto data = it.getData();
        _record.from_data(data.get());
        for (const auto &col_name : col_names)
        {
            auto entry = _record.entry(col_name);
            row.emplace_back(entry->to_string());
        }
        content.emplace_back(std::move(row));

        _content.erase(it);
    }

    return make_pair(std::move(col_names), std::move(content));
}

