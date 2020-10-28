//
// Created by Chestnut on 2020/11/21.
//

#ifndef OHMYDBMS_RECORDCHECKER_H
#define OHMYDBMS_RECORDCHECKER_H

#include "SQLTtree.h"
#include <memory>

class Record;

class RecordChecker
{
  public:
    virtual bool check(Record &record) = 0;
    virtual ~RecordChecker(){};
};

class TreeRecordChecker : public RecordChecker
{
    std::shared_ptr<SQLTree> _condition;

  public:
    TreeRecordChecker(std::shared_ptr<SQLTree> condition);

    bool check(Record &record);

    ~TreeRecordChecker()
    {
    }
};

class SimpleRecordChecker : public RecordChecker
{
    std::string col_name;
    std::string value;

  public:
    SimpleRecordChecker(std::string col_name,std::string value);

    bool check(Record &record);

    ~SimpleRecordChecker()
    {
    }
};

#endif // OHMYDBMS_RECORDCHECKER_H
