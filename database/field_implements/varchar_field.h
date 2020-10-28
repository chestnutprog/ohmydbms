//
// Created by Chestnut on 2020/11/16.
//

#ifndef OHMYDBMS_VARCHAR_FIELD_H
#define OHMYDBMS_VARCHAR_FIELD_H

class VARCHAR_Field : public BaseField
{
    int _max_length;
    struct _content_head
    {
        bool not_null;
        uint8_t _length;
    } content_head;
    std::unique_ptr<char> data;

    std::function<int(void)> bytes_size = [&]() { return sizeof(content_head) + _max_length; };
    std::function<int(void)> data_size = [&]() { return _max_length; };

    Column &_column;

  public:
    VARCHAR_Field(Column &column);

    static std::unique_ptr<BaseField> create(Column &column)
    {
        return std::make_unique<VARCHAR_Field>(column);
    }

    int size() override
    {
        return bytes_size();
    }

    bool not_null() override
    {
        return content_head.not_null;
    }

    void from_string(const std::optional<std::string> &str) override
    {
        if (str.has_value())
        {
            content_head.not_null = true;
            content_head._length = boost::numeric_cast<decltype(content_head._length)>(str.value().size());
            memset(data.get(), 0, data_size());
            str.value().copy(data.get(), content_head._length);
        }
        else
        {
            content_head.not_null = false;
            content_head._length = 0;
            memset(data.get(), 0, data_size());
        }
    }

    void from_bytes(const char *source) override
    {
        memcpy(&content_head, source, sizeof(content_head));
        memset(data.get(), 0, data_size());
        memcpy(data.get(), source + sizeof(content_head), content_head._length);
    }

    std::shared_ptr<char> to_bytes() const override
    {
        auto bytes = std::shared_ptr<char>((char *)malloc(bytes_size()));
        memset(bytes.get(), 0, bytes_size());
        memcpy(bytes.get(), &content_head, sizeof(content_head));
        memcpy(bytes.get() + sizeof(content_head), data.get(), data_size());
        return bytes;
    }

    std::optional<std::string> to_string() const override
    {
        if (content_head.not_null)
            return std::string(data.get(), content_head._length);
        return {};
    }

    bool operator<(const BaseField &_b) const override
    {
        if (auto b = dynamic_cast<const VARCHAR_Field *>(&_b))
        {
            to_string() < b->to_string();
        }
        return false;
    }

    virtual bool operator<(const std::string &b) const override
    {
        return to_string() < b;
    }

    virtual bool operator==(const std::string &b) const override
    {
        return to_string() == b;
    }
};

#endif // OHMYDBMS_VARCHAR_FIELD_H
