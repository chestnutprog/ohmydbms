//
// Created by Chestnut on 2020/11/6.
//

#ifndef OHMYDBMS_HELPER_H
#define OHMYDBMS_HELPER_H

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/functional/factory.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/range/combine.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/optional.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/unordered_set.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/bimap.hpp>
#include <boost/algorithm/string.hpp>

#include <fmt/format.h>

#include <boost/log/trivial.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <functional>

namespace fs = std::filesystem;

#define BOOL_TO_STR(x) (x?"true":"false")

#define SYNCED_MEMBER(type, name)                                                                                      \
  private:                                                                                                             \
    type name;                                                                                                         \
                                                                                                                       \
  public:                                                                                                              \
    type get##name()                                                                                                   \
    {                                                                                                                  \
        return name;                                                                                                   \
    }                                                                                                                  \
    void set##name(const type &t)                                                                                      \
    {                                                                                                                  \
        name = t;                                                                                                      \
        save();                                                                                                        \
    }

#define SYNCED_READONLY_MEMBER(type, name)                                                                             \
  private:                                                                                                             \
    type name;                                                                                                         \
                                                                                                                       \
  public:                                                                                                              \
    const type get##name()                                                                                             \
    {                                                                                                                  \
        return name;                                                                                                   \
    }

#define THIS ((type *)(this))

template <class type> class hold_file
{
  protected:
    std::fstream _file;

    std::fstream &getFile()
    {
        if (!_file.is_open())
        {
            _file.open(THIS->getFilePath(), std::ios::in | std::ios::out | std::ios::binary);
        }

        return _file;
    }

    std::fstream &getFileTouch()
    {
        if (!_file.is_open())
        {
            _file.open(THIS->getFilePath(), std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
        }
        return _file;
    }

    void closeFile()
    {
        _file.close();
    }

    void deleteFile()
    {
        if (_file.is_open())
            _file.close();
        fs::remove(THIS->getFilePath());
    }

  public:
    bool file_exist()
    {
        return fs::exists(THIS->getFilePath());
    }

    hold_file()
    {
        _file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    }
};

/**
 * A block file always have two part, head and content
 * content are made by blocks, and the 0 block is sentinel
 * node, which doesn't hold any data.
 *
 * @tparam type The Derived Class
 */
template <class type> class file_blocks : public hold_file<type>
{
    struct file_head_block
    {
        int formated_blocks;
        int next_free;
    } head;

    struct block_head
    {
        int prev;
        int next;
        char data[];
    };

    int _get_data_size()
    {
        return THIS->getBlockSize();
    }

    int _get_block_size()
    {
        return THIS->getBlockSize() + sizeof(block_head);
    }

    int block_to_bytes(int block_id)
    {
        return sizeof(head) + block_id * _get_block_size();
    }

    void read_head()
    {
        hold_file<type>::getFile().clear();
        hold_file<type>::getFile().seekg(0, std::ios::beg);
        hold_file<type>::getFile().read(&head, sizeof head);
    }

    void write_head()
    {
        hold_file<type>::getFileTouch().clear();
        hold_file<type>::getFile().seekp(0, std::ios::beg);
        hold_file<type>::getFile().write((char *)&head, sizeof head);
    }

    std::shared_ptr<block_head> empty_block()
    {
        auto p = malloc(_get_block_size());
        memset(p, 0, _get_block_size());
        return std::shared_ptr<block_head>((block_head *)p);
    }

    int get_free_block()
    {
        if (head.next_free)
            return head.next_free;
        return head.formated_blocks;
    }

    std::shared_ptr<block_head> readBlock(int pos)
    {
        auto block = empty_block();
        hold_file<type>::getFile().clear();
        hold_file<type>::getFile().seekg(block_to_bytes(pos), std::ios::beg);
        hold_file<type>::getFile().read((char *)block.get(), _get_block_size());
        return block;
    }

    void writeBlock(int pos, std::shared_ptr<block_head> data)
    {
        head.formated_blocks = std::max(head.formated_blocks, pos + 1);
        write_head();
        hold_file<type>::getFile().clear();
        hold_file<type>::getFile().seekp(block_to_bytes(pos), std::ios::beg);
        hold_file<type>::getFile().write((char *)data.get(), _get_block_size());
    }

  protected:
    void blocks_init()
    {
        head.formated_blocks = 1;
        head.next_free = 0;
        write_head();
        writeBlock(0, empty_block());
    }

  public:
    class iterator
    {
      protected:
        typedef iterator _Self;
        file_blocks &_blocks_file;
        int _node;

        friend class file_blocks;

        std::shared_ptr<block_head> _block;

        std::shared_ptr<block_head> get_block()
        {
            if (!_block)
            {
                _block = std::reinterpret_pointer_cast<block_head>(_blocks_file.readBlock(_node));
            }
            return _block;
        }

      public:
        iterator(file_blocks &blocks_file, int node) : _blocks_file(blocks_file), _node(node)
        {
        }

        std::shared_ptr<block_head> operator*()
        {
            return get_block();
        }

        bool operator==(const _Self &b)
        {
            return _node == b._node;
        }

        bool operator!=(const _Self &b)
        {
            return _node != b._node;
        }

        virtual _Self &operator++()
        {
            _node = get_block()->next;
            _block = nullptr;
            return *this;
        }

        virtual _Self &operator--()
        {
            _node = get_block()->prev;
            _block = nullptr;
            return *this;
        }

        _Self next()
        {
            return _Self(_blocks_file, get_block()->next);
        }

        _Self prev()
        {
            return _Self(_blocks_file, get_block()->prev);
        }

        void clear()
        {
            _block = nullptr;
        }

        void save()
        {
            _blocks_file.writeBlock(_node, get_block());
        }

        std::shared_ptr<char> getData()
        {
            std::shared_ptr<char> data = std::shared_ptr<char>((char *)malloc(_blocks_file._get_data_size()));
            memcpy(data.get(), &(get_block().get()->data), _blocks_file._get_data_size());
            return data;
        }

        void setData(char *data)
        {
            memcpy(&(get_block().get()->data), data, _blocks_file._get_data_size());
        }
    };

    iterator begin()
    {
        return iterator(*this, 0).next();
    }

    iterator end()
    {
        return iterator(*this, 0);
    }

    iterator insert(iterator &&it, char *data)
    {
        auto new_prev_it = it.prev();
        auto new_next_it = it;

        auto new_block = empty_block();
        new_block->prev = new_prev_it._node;
        new_block->next = new_next_it._node;
        auto new_pos = get_free_block();
        writeBlock(new_pos, new_block);
        auto new_it = iterator(*this, new_pos);
        new_it.setData(data);
        new_it.save();

        new_prev_it.clear();
        auto new_prev = *new_prev_it;
        new_prev->next = new_pos;
        new_prev_it.save();

        new_next_it.clear();
        auto new_next = *new_next_it;
        new_next->prev = new_pos;
        new_next_it.save();

        return new_it;
    }

    iterator erase(iterator &it)
    {
        auto prev_it = it.prev();
        auto next_it = it.next();

        prev_it.clear();
        auto new_prev = *prev_it;
        new_prev->next = next_it._node;
        prev_it.save();

        next_it.clear();
        auto new_next = *next_it;
        new_next->prev = prev_it._node;
        next_it.save();

        return next_it;
    }
};

template <class type> class archived_class : hold_file<type>
{
  public:
    void read()
    {
        hold_file<type>::getFile().clear();
        hold_file<type>::getFile().seekg(0, std::ios::beg);
        boost::archive::binary_iarchive iarch(hold_file<type>::getFile());
        iarch >> *THIS;
    }

    void save()
    {
        hold_file<type>::getFileTouch().clear();
        hold_file<type>::getFile().seekp(0, std::ios::beg);
        boost::archive::binary_oarchive oarch(hold_file<type>::getFile());
        oarch << *THIS;
    }

    void archive_del()
    {
        hold_file<type>::deleteFile();
    }

    bool archive_exist()
    {
        return hold_file<type>::file_exist();
    }
};

extern const fs::path basepath;
extern const fs::path datapath;
extern const fs::path logpath;

typedef std::pair<std::vector<std::string>, std::vector<std::vector<std::optional<std::string>>>> DB_RESULT_TYPE;

#endif // OHMYDBMS_HELPER_H
