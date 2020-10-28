//
// Created by Chestnut on 2020/11/16.
//

#ifndef OHMYDBMS_INTEGER_FIELD_H
#define OHMYDBMS_INTEGER_FIELD_H

class INTEGER_Field : public BaseField
{
    Column &_column;
    struct _content
    {
        int value;
        bool not_null;
    } content;

  public:
    INTEGER_Field(Column &column) : _column(column)
    {
        content.not_null = false;
    }

    static std::unique_ptr<BaseField> create(Column &column)
    {
        return std::make_unique<INTEGER_Field>(column);
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
        if (auto b = dynamic_cast<const INTEGER_Field *>(&_b))
        {
            return content.value < b->content.value;
        }
        return false;
    }

    virtual bool operator<(const std::string &b) const override
    {
        return content.value < std::stoi(b);
    }

    virtual bool operator==(const std::string &b) const override
    {
        return to_string() == b;
    }
};

#endif // OHMYDBMS_INTEGER_FIELD_H
