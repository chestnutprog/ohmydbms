//
// Created by Chestnut on 2020/11/10.
//

#ifndef OHMYDBMS_TABLE_H
#define OHMYDBMS_TABLE_H

#include "tablecontent.h"

#include "tablemetadata.h"

#include "../parser/recordchecker.h"

class Database;

class Table
{
    Database &_database;
    TableMetadata _metadata;
    const std::string _name;
    TableContent _content;

    friend class TableMetadata;
    friend class TableContent;
    friend class Record;

  public:
    Table(Database &database, const std::string &name);

    void create(const std::vector<std::tuple<std::string, Column>> &cols);

    void add_col(std::string, Column);

    void del_col(std::string);

    void rename_col(const std::string &before, const std::string &after);

    void del();

    bool exist();

    TableContent &content()
    {
        return _content;
    }

    DB_RESULT_TYPE desc();

    DB_RESULT_TYPE select();

    DB_RESULT_TYPE select(std::shared_ptr<RecordChecker> checker);

    DB_RESULT_TYPE erase(std::shared_ptr<RecordChecker> checker);

    DB_RESULT_TYPE update(std::shared_ptr<RecordChecker> checker, std::vector<std::shared_ptr<RecordChecker>> updaters);
};

#endif // OHMYDBMS_TABLE_H
