//
// Created by Chestnut on 2020/11/10.
//

#include "column.h"

Column::Column(FieldType type, boost::optional<short> length, bool not_null, bool unique, bool primary,
               boost::optional<std::string> default_value)
    : type(type), _length(length), not_null(not_null), unique(unique), primary(primary), default_value(default_value)
{
    if (type == FieldType::VARCHAR && !length.has_value())
        throw std::invalid_argument("VARCHAR must has a length limit!");
}

std::unique_ptr<BaseField> Column::getField()
{
    return BaseField::get(type, *this);
}
