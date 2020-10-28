//
// Created by Chestnut on 2020/11/4.
//

#ifndef OHMYDBMS_DATABASE_H
#define OHMYDBMS_DATABASE_H

#include "../helper.h"
#include "table.h"

class Database;

class Table;

class DatabaseList : archived_class<DatabaseList>, boost::serialization::singleton<DatabaseList>
{
    std::unordered_set<std::string> _dbs;
};

class Database
{
  private:
    std::string _name;
    std::unordered_map<std::string, Table> _tables;

    friend class Table;

  public:
    const fs::path _database_path;

    const std::string &name()
    {
        return _name;
    }

    Database(std::string name);

    void create();

    void del();

    void del_if_exist();

    bool exist();

    void rename(const std::string &name);

    void rename_table(const std::string &old_name, const std::string &new_name);

    Table &table(const std::string &name);

    std::vector<std::string> table_list();

    DB_RESULT_TYPE show_table();

    static std::vector<std::string> database_list();

    static DB_RESULT_TYPE show_database();
};

#endif // OHMYDBMS_DATABASE_H
