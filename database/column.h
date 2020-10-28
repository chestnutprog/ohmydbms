//
// Created by Chestnut on 2020/11/10.
//

#ifndef OHMYDBMS_COLUMN_H
#define OHMYDBMS_COLUMN_H

#include "../helper.h"
#include "field.h"

class Column
{
    FieldType type;
    boost::optional<short> _length;
    bool not_null;
    bool unique;
    bool primary;
    boost::optional<std::string> default_value;

    friend class boost::serialization::access;
    template <class T1, class T2> friend class std::pair;
    friend class Record;
    friend class BaseField;
    friend class Table;
    friend class TableContent;

    template <class Archive> void serialize(Archive &ar, const unsigned int version)
    {
        ar &type;
        ar &_length;
        ar &not_null;
        ar &unique;
        ar &primary;
        ar &default_value;
    }

  public:
    Column(FieldType type, boost::optional<short> length, bool not_null = false, bool unique = false,
           bool primary = false, boost::optional<std::string> default_value = boost::none);

    Column()
    {
    }

    std::unique_ptr<BaseField> getField();

    const boost::optional<short> &getMaxLength()
    {
        return _length;
    }
};

#endif // OHMYDBMS_COLUMN_H
