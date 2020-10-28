//
// Created by Chestnut on 2020/11/25.
//

#ifndef OHMYDBMS_RECORDUPDATER_H
#define OHMYDBMS_RECORDUPDATER_H

#include "SQLTtree.h"
#include <memory>

class Record;

class RecordUpdater
{
  public:
    virtual void update(Record &record) = 0;
    virtual ~RecordUpdater(){};
};

class TreeRecordUpdater : public RecordUpdater
{
    std::shared_ptr<SQLTree> _condition;

  public:
    TreeRecordUpdater(std::shared_ptr<SQLTree> condition);

    void update(Record &record);

    ~TreeRecordUpdater()
    {
    }
};

#endif // OHMYDBMS_RECORDUPDATER_H
