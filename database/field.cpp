//
// Created by Chestnut on 2020/11/11.
//

#include "field.h"

decltype(BaseField::_factory) BaseField::_factory;

BaseField::Register<FieldType::INTEGER, INTEGER_Field> _reg_INTEGER_Field;
BaseField::Register<FieldType::VARCHAR, VARCHAR_Field> _reg_VARCHAR_Field;
// BaseField::Register<FieldType::BOOLEAN, BOOLEAN_Field> _reg_BOOLEAN_Field;
// BaseField::Register<FieldType::DATETIME, DATETIME_Field> _reg_DATETIME_Field;

std::string BaseField::type_to_string(FieldType type)
{
    static std::unordered_map<FieldType, std::string> _m{
        {FieldType::INTEGER, std::string("INT")}, {FieldType::VARCHAR, std::string("VARCHAR")},
        //                                                         {FieldType::DATETIME, std::string("DATETIME")},
        //                                                         {FieldType::BOOLEAN, std::string("BOOLEAN")}
    };
    return _m.at(type);
}

FieldType BaseField::string_to_type(std::string type)
{
    boost::algorithm::to_upper(type);
    static std::unordered_map<std::string, FieldType> _m{
        {std::string("INT"), FieldType::INTEGER},
        {std::string("VARCHAR"), FieldType::VARCHAR},
        {std::string("INTEGER"), FieldType::INTEGER}
        //                                                         {FieldType::DATETIME, std::string("DATETIME")},
        //                                                         {FieldType::BOOLEAN, std::string("BOOLEAN")}
    };
    return _m.at(type);
}
