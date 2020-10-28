//
// Created by Chestnut on 2020/11/10.
//

#ifndef OHMYDBMS_TABLEMETADATA_H
#define OHMYDBMS_TABLEMETADATA_H

#include "column.h"

class Table;
class TableContent;

class TableMetadata : archived_class<TableMetadata>, boost::serialization::singleton<TableMetadata>
{
    const Table &_table;
    boost::bimap<int, std::string> col_names;
    std::vector<Column> cols;

    friend class boost::serialization::access;

    friend class TableContent;
    friend class Record;
    friend class Table;

    template <class Archive> void serialize(Archive &ar, const unsigned int version)
    {
        ar &cols;
        ar &col_names;
    }

  public:
    TableMetadata(const Table &table);

    void init();

    void create(const std::vector<std::tuple<std::string, Column>> &cols);

    bool exist();

    void del();

    fs::path getFilePath();
};

#endif // OHMYDBMS_TABLEMETADATA_H
