//
// Created by Chestnut on 2020/11/4.
//

#include "database.h"
#include "../helper.h"

Database::Database(std::string name) : _name{name}, _database_path(datapath / name)
{
}

Table &Database::table(const std::string &name)
{
    if (!exist())
    {
        BOOST_LOG_TRIVIAL(warning) << fmt::format("Database {} doesn't exists!", _name);
        throw std::invalid_argument(fmt::format("Database {} doesn't exists!", _name));
    }
    if (!_tables.count(name))
        _tables.try_emplace(name, *this, name);
    return _tables.at(name);
}

void Database::create()
{
    BOOST_LOG_TRIVIAL(info) << fmt::format("Create database {}.", _name);
    if (exist())
    {
        BOOST_LOG_TRIVIAL(warning) << fmt::format("Database {} already exists!", _name);
        throw std::invalid_argument(fmt::format("Database {} already exists!", _name));
    }
    fs::create_directory(datapath / _name);
    BOOST_LOG_TRIVIAL(info) << fmt::format("Database {} created.", _name);
}

void Database::del()
{
    BOOST_LOG_TRIVIAL(info) << fmt::format("Drop database {}.", _name);
    if (!exist())
    {
        BOOST_LOG_TRIVIAL(warning) << fmt::format("Database {} doesn't exists!", _name);
        throw std::invalid_argument(fmt::format("Database {} doesn't exists!", _name));
    }
    fs::remove_all(datapath / _name);
    BOOST_LOG_TRIVIAL(info) << fmt::format("Database {} dropped.", _name);
}

bool Database::exist()
{
    return fs::exists(datapath / _name);
}

void Database::del_if_exist()
{
    if (exist())
        del();
}

std::vector<std::string> Database::table_list()
{
    if (!exist())
    {
        BOOST_LOG_TRIVIAL(warning) << fmt::format("Database {} doesn't exists!", _name);
        throw std::invalid_argument(fmt::format("Database {} doesn't exists!", _name));
    }
    std::vector<std::string> list;
    for (auto &it : fs::recursive_directory_iterator(_database_path))
    {
        if (it.path().extension() == ".db")
        {
            list.emplace_back(it.path().stem());
        }
    }
    return list;
}

DB_RESULT_TYPE Database::show_table()
{
    std::vector<std::string> col_names = {"table"};
    std::vector<std::vector<std::optional<std::string>>> content;
    auto list = table_list();
    for (auto name : list)
    {
        std::vector<std::optional<std::string>> row;
        row.emplace_back(name);
        content.emplace_back(std::move(row));
    }
    return make_pair(std::move(col_names), std::move(content));
}

std::vector<std::string> Database::database_list()
{
    std::vector<std::string> list;
    for (auto &it : fs::directory_iterator(datapath))
    {
        if (it.is_directory())
        {
            list.emplace_back(it.path().filename());
        }
    }
    return list;
}

DB_RESULT_TYPE Database::show_database()
{
    std::vector<std::string> col_names = {"database"};
    std::vector<std::vector<std::optional<std::string>>> content;
    auto list = database_list();
    for (auto name : list)
    {
        std::vector<std::optional<std::string>> row;
        row.emplace_back(name);
        content.emplace_back(std::move(row));
    }
    return make_pair(std::move(col_names), std::move(content));
}

void Database::rename(const std::string &name)
{
    BOOST_LOG_TRIVIAL(info) << fmt::format("Rename database {} to {}.", _name, name);
    if (!exist())
    {
        BOOST_LOG_TRIVIAL(warning) << fmt::format("Database {} doesn't exists!", _name);
        throw std::invalid_argument(fmt::format("Database {} doesn't exists!", _name));
    }
    Database _new_database(name);
    if (_new_database.exist())
    {
        BOOST_LOG_TRIVIAL(warning) << fmt::format("Database {} already exists!", name);
        throw std::invalid_argument(fmt::format("Database {} already exists!", name));
    }

    fs::rename(datapath / _name, datapath / name);
    _name = name;

    BOOST_LOG_TRIVIAL(info) << fmt::format("Database {} renamed.", _name);
}

void Database::rename_table(const std::string &old_name, const std::string &new_name)
{
    Table &old_table = table(old_name);
    if (!exist())
    {
        BOOST_LOG_TRIVIAL(warning) << fmt::format("Table {}.{} doesn't exists!", _name, old_name);
        throw std::invalid_argument(fmt::format("Table {}.{} doesn't exists!", _name, old_name));
    }
    Table &new_table = table(new_name);
    if (new_table.exist())
    {
        BOOST_LOG_TRIVIAL(warning) << fmt::format("Table {}.{} already exists!", _name, new_name);
        throw std::invalid_argument(fmt::format("Table {}.{} already exists!", _name, new_name));
    }

    for (auto &it : fs::recursive_directory_iterator(_database_path))
    {
        if (it.path().stem() == old_name)
        {
            fs::rename(it.path(), new_name + it.path().extension().string());
        }
    }
}
