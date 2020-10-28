//
// Created by Chestnut on 2020/11/10.
//

#ifndef OHMYDBMS_RECORD_H
#define OHMYDBMS_RECORD_H

#include "table.h"

class Record
{
    Table &_table;
    std::map<std::string, std::unique_ptr<BaseField>> entries;

    std::unordered_map<std::string, int> _bytes_offset;

  public:
    Record(Table &table);

    std::shared_ptr<char> to_bytes();

    BaseField *entry(const std::string &name);

    void validation();

    void from_data(char *data);

    int getBlockSize();
};

#endif // OHMYDBMS_RECORD_H
