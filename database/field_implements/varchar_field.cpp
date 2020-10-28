//
// Created by Chestnut on 2020/11/17.
//

#include "../column.h"
#include "../field.h"

#include "varchar_field.h"

VARCHAR_Field::VARCHAR_Field(Column &column) : _column(column)
{
    _max_length = _column.getMaxLength().value();
    memset(&content_head, 0, sizeof content_head);
    data = std::unique_ptr<char>((char *)malloc(data_size()));
}