//
// Created by Chestnut on 2020/11/11.
//

#ifndef OHMYDBMS_FIELD_H
#define OHMYDBMS_FIELD_H

#include "../helper.h"

class Column;

enum class FieldType
{
    INTEGER,
    VARCHAR,
    TEXT,
    DATETIME,
    BOOLEAN
};

template <FieldType T, class U> class Field_Register;

class BaseField
{
    static std::unordered_map<FieldType, std::function<std::unique_ptr<BaseField>(Column &)>> _factory;

  public:
    template <FieldType T, class U> struct Register
    {
        Register()
        {
            _factory[T] = &U::create;
        }
    };

    virtual int size() = 0;
    virtual bool not_null() = 0;
    virtual void from_string(const std::optional<std::string> &str) = 0;
    virtual void from_bytes(const char *source) = 0;
    virtual std::shared_ptr<char> to_bytes() const = 0;
    virtual std::optional<std::string> to_string() const = 0;
    virtual bool operator<(const BaseField &b) const = 0;
    virtual bool operator<(const std::string &b) const = 0;
//    virtual bool operator>(const BaseField &b) const = 0;
//    virtual bool operator>(const std::string &b) const = 0;
//    virtual bool operator==(const BaseField &b) const = 0;
    virtual bool operator==(const std::string &b) const = 0;
    virtual ~BaseField()
    {
    }

    static std::unique_ptr<BaseField> get(FieldType type, Column &column)
    {
        auto gen = _factory.at(type);
        return gen(column);
    }

    static std::string type_to_string(FieldType type);

    static FieldType string_to_type(std::string type);
};

#include "field_implements/field_implements.h"

#endif // OHMYDBMS_FIELD_H
