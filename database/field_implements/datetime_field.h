//
// Created by Chestnut on 2020/11/19.
//

#ifndef OHMYDBMS_DATETIME_FIELD_H
#define OHMYDBMS_DATETIME_FIELD_H

class DATETIME_Field : public BaseField
{
    Column &_column;
    struct _content
    {
        int value;
        bool not_null;
    } content;

  public:
    DATETIME_Field(Column &column) : _column(column)
    {
        content.not_null = false;
    }

    static std::unique_ptr<BaseField> create(Column &column)
    {
        return std::make_unique<DATETIME_Field>(column);
    }

    bool not_null() override
    {
        return content.not_null;
    }

    int size() override
    {
        return sizeof(content);
    }

    void from_string(const std::optional<std::string> &str) override
    {
        if (str.has_value())
        {
            content.not_null = true;
            content.value = std::stoi(str.value());
        }
        else
        {
            content.not_null = false;
        }
    }

    void from_bytes(const char *source) override
    {
        memcpy(&content, source, sizeof(content));
    }

    std::shared_ptr<char> to_bytes() const override
    {
        auto bytes = std::shared_ptr<char>((char *)malloc(sizeof(content)));
        memcpy(bytes.get(), &content, sizeof(content));
        return bytes;
    }

    std::optional<std::string> to_string() const override
    {
        if (content.not_null)
            return std::to_string(content.value);
        return {};
    }

    bool operator<(const BaseField &_b) const override
    {
        if (auto b = dynamic_cast<const DATETIME_Field *>(&_b))
        {
            return content.value < b->content.value;
        }
        return false;
    }
};

#endif // OHMYDBMS_DATETIME_FIELD_H
