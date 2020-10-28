//
// Created by Chestnut on 2020/11/11.
//

#ifndef OHMYDBMS_TABLECONTENT_H
#define OHMYDBMS_TABLECONTENT_H

#include "../helper.h"
#include "../parser/recordchecker.h"

class Table;
class RecordChecker;
class SQLTree;

class TableContent : file_blocks<TableContent>
{
    friend class Table;
    friend class Record;

    Table &_table;

    void create();

    void del();

    TableContent(Table &table);

  public:
    fs::path getFilePath();

    int getBlockSize();

    void insert(const std::vector<std::tuple<std::string, std::optional<std::string>>> &values);

    class range_iterator : public iterator
    {
        typedef range_iterator _Self;
        Table &_table;
        std::shared_ptr<RecordChecker> _checker;

      public:
        range_iterator(file_blocks &blocksFile, int node, Table &table, std::shared_ptr<RecordChecker> checker)
            : iterator(blocksFile, node), _table(table), _checker(checker)
        {
        }

        _Self &operator++() override;

        _Self &operator--() override;

        iterator next() = delete;

        iterator prev() = delete;
    };

    range_iterator range_begin(std::shared_ptr<RecordChecker> checker);

    range_iterator end();
};

#endif // OHMYDBMS_TABLECONTENT_H
