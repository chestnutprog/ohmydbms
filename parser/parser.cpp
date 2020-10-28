//
// Created by Chestnut on 2020/11/4.
//

#include "parser.h"
#include "../database/database.h"

#include "../sys/User.h"
#include "../sys/log.h"
#include "recordchecker.h"

#include <algorithm>
#include <iostream>
#include <map>
#include <regex>
#include <stack>
#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace parse
{

std::string remove_quotes(const std::string &str)
{
    if (str.at(0) == '\'' || str.at(0) == '"')
    {
        return str.substr(1, str.size() - 2);
    }
    return str;
}

int string_length;
SQLTree *getCondition(std::string s);
DB_RESULT_TYPE forCreate(string s);
DB_RESULT_TYPE forCreateUser(string s);
DB_RESULT_TYPE forCreateDatabase(string s);
DB_RESULT_TYPE forCreateTable(string s);
DB_RESULT_TYPE forAlter(string s);
DB_RESULT_TYPE forAlterUser(string s);
DB_RESULT_TYPE forAlterDatabase(string s);
DB_RESULT_TYPE forAlterTable(string s);
DB_RESULT_TYPE forShow(string s);
DB_RESULT_TYPE forDrop(string s);
DB_RESULT_TYPE forDel(string s);
DB_RESULT_TYPE forInsert(string s);
DB_RESULT_TYPE forUpdate(string s);
DB_RESULT_TYPE forSelect(string s);
DB_RESULT_TYPE forAuthorization(std::string s);
DB_RESULT_TYPE forDesc(string s);
vector<string> getDatabaseAndTable(string s);
vector<string> getline(string line, string type, string sep);
string remove(string s, int length);
string forEnd(std::string s);
std::vector<std::vector<std::string>> getColumns(std::string columns);

const unordered_map<string, string> item_type = {
    {"database", "([a-zA-Z]\\w*)"},
    {"table", "([a-zA-Z]\\w*)"},
    {"user", "([a-zA-Z]\\w*)"},
    {"column_name", "([a-zA-Z]\\w*)"},
    {"column_type", "(int|integer|bool|double|varchar[(]\\d+[)]|datetime)"},
    {"value", "([0-9]+|[0-9]+[.][0-9]+|true|false|['].+?[']|[\"].+?[\"])"},
    {"password", "(\\w+)"}};

string remove(string s, int remove_length)
{
    if (s.length() < remove_length)
    {
        throw runtime_error("can't remove from string '" + s + "' with length " + to_string(remove_length));
    }
    string_length += remove_length;
    return s.substr(remove_length, s.length());
}

DB_RESULT_TYPE forAuthorization(std::string s)
{
    //存储单元
    string operation;
    vector<string> database;
    vector<string> user;
    smatch result;
    //获取类型
    if (regex_search(s.cbegin(), s.cend(), result, regex("^\\s*(revoke|grant)\\s", regex::icase)))
    {
        operation = result[1];
        transform(operation.begin(), operation.end(), operation.begin(), ::toupper);
        s = remove(s, result[0].length());
    }
    else
    {
        throw runtime_error("invalid operation.");
    }
    //获取数据库
    while (regex_search(s.cbegin(), s.cend(), result, regex("^\\s*" + item_type.find("database")->second + "\\s*,")))
    {
        database.push_back(result[1]);
        s = remove(s, result[0].length());
    }
    if (regex_search(s.cbegin(), s.cend(), result, regex("^\\s*" + item_type.find("database")->second + "\\s+to\\s")))
    {
        database.push_back(result[1]);
        s = remove(s, result[0].length());
    }
    else
    {
        throw runtime_error("invalid authorization operation. unrecognizable database name or missing 'to'.");
    }
    //读取用户名
    while (regex_search(s.cbegin(), s.cend(), result, regex("^\\s*" + item_type.find("user")->second + "\\s*,")))
    {
        user.push_back(result[1]);
        s = remove(s, result[0].length());
    }
    if (regex_search(s.cbegin(), s.cend(), result, regex("^\\s*" + item_type.find("user")->second + "\\s*")))
    {
        user.push_back(result[1]);
        s = remove(s, result[0].length());
    }
    else
    {
        throw runtime_error("invalid authorization operation,require user as input.");
    }
    //确认结束
    forEnd(s);
    //输出结果，调试用
    cout << "operation  " << operation << endl;
    cout << "database";
    for (int i = 0; i < database.size(); i++)
    {
        cout << "  " << database[i];
    }
    cout << endl;
    cout << "user";
    for (int i = 0; i < user.size(); i++)
    {
        cout << "  " << user[i];
    }
    cout << endl;
    return {};
}

// desc <database name>.<table name>;
DB_RESULT_TYPE forDesc(std::string s)
{
    string database;
    string table;
    smatch result;

    regex_search(s.cbegin(), s.cend(), result, regex("^\\s*desc\\s", regex::icase));
    s = remove(s, result[0].length());
    vector<string> tempo = getDatabaseAndTable(s);
    database = tempo[0];
    table = tempo[1];
    s = tempo[2];
    forEnd(s);
    cout << database << "  " << table << endl;

    Database _database(database);
    return _database.table(table).desc();
}

//    create user <user name> identified by <password>;
//    create database <database name>;
//    Create table <table name> to <database> （column_name colums_type(size) 主键/外键,唯一,非空 ...）;
DB_RESULT_TYPE forCreate(string s)
{
    string type;
    smatch result;
    regex_search(s.cbegin(), s.cend(), result, regex("^\\s*create\\s", regex::icase));
    s = remove(s, result[0].length());
    if (regex_search(s.cbegin(), s.cend(), result, regex("^\\s*(user|database|table)[\\W]", regex::icase)))
    {
        type = result[1];
        transform(type.begin(), type.end(), type.begin(), ::toupper);
        s = remove(s, result[0].length());
        if (type == "USER")
        {
            return forCreateUser(s);
        }
        else if (type == "DATABASE")
        {
            return forCreateDatabase(s);
        }
        else if (type == "TABLE")
        {
            return forCreateTable(s);
        }
    }
    else
    {
        throw runtime_error("inavlid create operation.");
    }
    return {};
}

DB_RESULT_TYPE forCreateUser(string s)
{
    string user;
    string password;
    smatch result;
    //获得用户名
    if (regex_search(s.cbegin(), s.cend(), result,
                     regex("^\\s*" + item_type.find("user")->second + "\\s+identified\\s+by\\s*", regex::icase)))
    {
        user = result[1];
        s = remove(s, result[0].length());
    }
    else
    {
        throw runtime_error("invalid create user operation. user name unrecognizable or missing 'identified by'.");
    }
    //获得密码
    if (regex_search(s.cbegin(), s.cend(), result, regex("^\\s*" + item_type.find("password")->second + "\\s*;")))
    {
        password = result[1];
        s = remove(s, result[0].length() - 1);
    }
    else
    {
        throw runtime_error("invalid create user operation. password unrecognizable.");
    }
    //确认结束
    s = forEnd(s);
    //输出结果，测试用
    cout << user << "  " << password << endl;
    UserList::get_mutable_instance().new_user(user, password);
    return {};
}

DB_RESULT_TYPE forCreateDatabase(string s)
{
    string database;
    smatch result;
    //获得库名
    if (regex_search(s.cbegin(), s.cend(), result, regex("^\\s*" + item_type.find("database")->second + "\\s*")))
    {
        database = result[1];
        s = remove(s, result[0].length());
    }
    else
    {
        throw runtime_error("invalid create database operation. database name unrecognizable.");
    }
    //确认结束
    s = forEnd(s);

    //    cout << database << endl;
    Database _database(database);
    _database.create();
    return {};
}

DB_RESULT_TYPE forCreateTable(string s)
{
    string table;
    string database;
    vector<vector<string>> columns = vector<vector<string>>();
    smatch result;

    //获得表名
    if (regex_search(s.cbegin(), s.cend(), result,
                     regex("^\\s*" + item_type.find("table")->second + "\\s+to\\s*", regex::icase)))
    {
        table = result[1];
        s = remove(s, result[0].length());
    }
    else
    {
        throw runtime_error("invalid create table operation. table name unrecognizable or missing 'to'.");
    }
    //获得库名
    if (regex_search(s.cbegin(), s.cend(), result, regex("^\\s*" + item_type.find("database")->second + "\\s*")))
    {
        database = result[1];
        s = remove(s, result[0].length());
    }
    else
    {
        throw runtime_error("invalid create table operation. database name unrecognizable.");
    }
    //获得列描述(此处还可优化,位置计数...)
    if (regex_search(s.cbegin(), s.cend(), result, regex("^(\\s*)[(](.*)[)]\\s*")))
    {
        string_length += result[1].length() + 1;
        columns = getColumns(result[2]);
        s = s.substr(result[0].length(), s.length());
        string_length += 1;
    }
    //确认结束
    s = forEnd(s);

    //输出结果，测试用
    cout << table << "  " << database << endl;
    for (int i = 0; i < columns.size(); i++)
    {
        for (int j = 0; j < columns[i].size(); j++)
        {
            cout << columns[i][j] << "  ";
        }
        cout << endl;
    }

    Database _database(database);
    auto &&_table = _database.table(table);
    std::vector<std::tuple<std::string, Column>> cols;

    for (auto &col : columns)
    {
        std::string col_name = col[0];
        FieldType type = BaseField::string_to_type(col[1]);

        boost::optional<short> length;
        bool unique = false;
        bool primary = false;
        bool not_null = false;
        boost::optional<std::string> default_value;

        for (int j = 2; j < col.size(); j++)
        {
            if (col[j] == "unique")
            {
                unique = true;
            }
            else if (col[j] == "primary")
            {
                primary = true;
            }
            else if (col[j] == "not null")
            {
                not_null = true;
            }
        }

        Column c{type, length, not_null, unique, primary};
        cols.emplace_back(col_name, c);
    }

    _table.create(cols);

    return {{}, {}};
}

//    alter user <user name> identified by <password>;
//    alter database <database name> rename <database name>;
//    alter table <database name>.<table name> rename <table name>;
//    alter table <database name>.<table name> modify <column_name>（<column_name> <column_type>
//    主键/外键，唯一，非空）; alter table <database name>.<table name> drop <column_name> ; alter table <database
//    name>.<table name> add <column_name> <column_type> 主键/外键，唯一，非空;
DB_RESULT_TYPE forAlter(string s)
{
    string type;
    smatch result;
    regex_search(s.cbegin(), s.cend(), result, regex("^\\s*alter\\s", regex::icase));
    s = remove(s, result[0].length());
    if (regex_search(s.cbegin(), s.cend(), result, regex("^\\s*(user|database|table)\\s", regex::icase)))
    {
        type = result[1];
        transform(type.begin(), type.end(), type.begin(), ::toupper);
        s = remove(s, result[0].length());
        if (type == "USER")
        {
            return forAlterUser(s);
        }
        else if (type == "DATABASE")
        {
            return forAlterDatabase(s);
        }
        else if (type == "TABLE")
        {
            return forAlterTable(s);
        }
    }
    else
    {
        throw runtime_error("invalid alter operation.");
    }
    return {{}, {}};
}

DB_RESULT_TYPE forAlterUser(string s)
{
    string user;
    string password;
    smatch result;
    //获得用户名
    if (regex_search(s.cbegin(), s.cend(), result,
                     regex("^\\s*" + item_type.find("user")->second + "\\s+\\identified\\s+by\\s*")))
    {
        user = result[1];
        s = remove(s, result[0].length());
    }
    else
    {
        throw runtime_error("invalid alter user operation. user name unrecognizable or missing 'identified by'.");
    }
    //获得密码
    if (regex_search(s.cbegin(), s.cend(), result, regex("^\\s*" + item_type.find("password")->second + "\\s*")))
    {
        password = result[1];
        s = remove(s, result[0].length());
    }
    else
    {
        throw runtime_error("invalid alter user operation. password unrecognizable.");
    }
    //确认结束
    s = forEnd(s);

    //输出结果，测试用
    cout << user << "  " << password << endl;
    return {};
}

DB_RESULT_TYPE forAlterDatabase(string s)
{
    string database;
    string newName;
    smatch result;
    //获得库名
    if (regex_search(s.cbegin(), s.cend(), result,
                     regex("^\\s*" + item_type.find("database")->second + "\\s+rename\\s*", regex::icase)))
    {
        database = result[1];
        s = remove(s, result[0].length());
    }
    else
    {
        throw runtime_error("invalid alter database operation. database name unrecognizable or missing 'rename'.");
    }
    //获得新名称
    if (regex_search(s.cbegin(), s.cend(), result, regex("^\\s*" + item_type.find("database")->second)))
    {
        newName = result[1];
        s = remove(s, result[0].length());
    }
    else
    {
        throw runtime_error("invalid alter database operation. new name unrecognizable.");
    }
    //确认结束
    s = forEnd(s);

    cout << database << "  " << newName << endl;

    Database _database(database);
    _database.rename(newName);

    return {};
}

DB_RESULT_TYPE forAlterTable(string s)
{
    string table;
    string database;
    string type;
    string name;
    vector<string> column = vector<string>();
    smatch result;

    //获得数据库名
    if (regex_search(s.cbegin(), s.cend(), result, regex("^\\s*([\\w\\_]+)\\s*")))
    {
        database = result[1];
        s = remove(s, result[0].length());
    }
    else
    {
        throw runtime_error("invalid alter table operation. database name unrecognizable.");
    }
    //中间标志符
    if (regex_search(s.cbegin(), s.cend(), result, regex("^\\s*.\\s*", regex::icase)))
    {
        s = remove(s, result[0].length());
    }
    else if (regex_search(s.cbegin(), s.cend(), result, regex("\\s*.\\s", regex::icase)))
    {
        throw runtime_error("invalid alter table operation. duplicate input before '.'.");
    }
    else
    {
        throw runtime_error("invalid alter table operation. missing '.'.");
    }
    //获得表名
    if (regex_search(s.cbegin(), s.cend(), result, regex("^\\s*([\\w\\_]+)\\s")))
    {
        table = result[1];
        s = remove(s, result[0].length());
    }
    else
    {
        throw runtime_error("invalid alter table operation. table name unrecognizable.");
    }
    //操作类型
    if (regex_search(s.cbegin(), s.cend(), result, regex("^\\s*(rename|modify|drop|add)\\s")))
    {
        type = result[1];
        transform(type.begin(), type.end(), type.begin(), ::toupper);
        s = remove(s, result[0].length());
    }
    else
    {
        throw runtime_error("invalid alter table operation. table name unrecognizable.");
    }
    if (type == "ADD")
    {
        if (regex_search(s.cbegin(), s.cend(), result, regex("^(.+);")))
        {
            vector<vector<string>> columns = getColumns(result[1]);
            if (columns.size() > 1)
            {
                throw runtime_error("invalid alter table operation.more than one column description");
            }
            else
            {
                column = columns[0];
            }
            name = column[0];
            s = s.substr(result[0].length() - 1, s.length());
        }
        else
        {
            throw runtime_error("invalid alter table operation. column description unrecognizable.");
        }
    }
    else
    {
        //获得名称
        if (regex_search(s.cbegin(), s.cend(), result, regex("^\\s*([\\w\\_]+)\\s*")))
        {
            name = result[1];
            s = remove(s, result[0].length());
        }
        else
        {
            throw runtime_error("invalid alter table operation. operation name unrecognizable.");
        }
        //获得列描述(此处还可优化,位置计数...)
        if (type == "MODIFY")
        {
            if (regex_search(s.cbegin(), s.cend(), result, regex("^(\\s*)[(](.+)[)]")))
            {
                string_length += result[1].length();
                vector<vector<string>> columns = getColumns(result[2]);
                if (columns.size() > 1)
                {
                    throw runtime_error("invalid alter table operation.more than one column description");
                }
                else
                {
                    column = columns[0];
                }
                s = s.substr(result[0].length(), s.length());
                string_length += 1;
            }
            else if (regex_search(s.cbegin(), s.cend(), result, regex("\\s*[(](.+)[)]")))
            {
                throw runtime_error("invalid alter table operation. duplicate input before column description.");
            }
            else
            {
                throw runtime_error("invalid alter table operation. column description unrecognizable.");
            }
        }
    }
    //确认结束
    s = forEnd(s);

    //输出结果，测试用
    cout << database << "  " << table << "  " << type << "  " << name << endl;
    for (int i = 0; i < column.size(); i++)
    {
        cout << column[i] << "  ";
    }
    cout << endl;
    return {};
}

//    show database;
//    SHOW TABLES IN <database name>;
//    SHOW log;
DB_RESULT_TYPE forShow(string s)
{
    string type;
    string database = "";
    smatch result;
    regex_search(s.cbegin(), s.cend(), result, regex("^\\s*show\\s", regex::icase));
    s = remove(s, result[0].length());
    if (regex_search(s.cbegin(), s.cend(), result, regex("^\\s*(database|log)", regex::icase)))
    {
        type = result[1];
        transform(type.begin(), type.end(), type.begin(), ::toupper);
        s = remove(s, result[0].length());
    }
    else if (regex_search(s.cbegin(), s.cend(), result, regex("^\\s*(tables)\\s+(in)\\s", regex::icase)))
    {
        string tempo1 = result[1];
        string tempo2 = result[2];
        type = tempo1 + " " + tempo2;
        transform(type.begin(), type.end(), type.begin(), ::toupper);
        s = remove(s, result[0].length());
        if (regex_search(s.cbegin(), s.cend(), result, regex("^\\s*([\\w\\_]+)")))
        {
            database = result[1];
            s = remove(s, result[0].length());
        }
        else
        {
            throw runtime_error("invalid show tables operation. database name unrecognizable.");
        }
    }
    else
    {
        throw runtime_error("invalid show operation.");
    }
    s = forEnd(s);

    //    cout << type << "  " << database << endl;
    if (type == "DATABASE")
    {
        return Database::show_database();
    }
    else if (type == "TABLES IN")
    {
        Database _database(database);
        return _database.show_table();
    }
    else if (type == "LOG")
    {
        return Logger::show_log();
    }
}

//    drop database <database name>;
//    drop table <database name>.<table name>;
DB_RESULT_TYPE forDrop(string s)
{
    string type;
    string database;
    string table = "";
    smatch result;
    regex_search(s.cbegin(), s.cend(), result, regex("^\\s*drop\\s", regex::icase));
    s = remove(s, result[0].length());
    if (regex_search(s.cbegin(), s.cend(), result, regex("^\\s*(database|table)\\s", regex::icase)))
    {
        type = result[1];
        transform(type.begin(), type.end(), type.begin(), ::toupper);
        s = remove(s, result[0].length());
    }
    else
    {
        throw runtime_error("invalid drop operation.");
    }
    //获得数据库名
    if (type == "DATABASE")
    {
        if (regex_search(s.cbegin(), s.cend(), result, regex("^\\s*([\\w\\_]+)")))
        {
            database = result[1];
            s = remove(s, result[0].length());
        }
        else
        {
            throw runtime_error("invalid drop database operation. database name unrecognizable.");
        }
    }
    else if (type == "TABLE")
    {
        vector<string> tempo = getDatabaseAndTable(s);
        database = tempo[0];
        table = tempo[1];
        s = tempo[2];
    }
    else
    {
        throw runtime_error("function error. invalid operation type");
    }
    s = forEnd(s);
    cout << type << "  " << database << "  " << table << endl;
    Database _database(database);
    if (type == "DATABASE")
    {
        _database.del();
    }
    else if (type == "TABLE")
    {
        _database.table(table).del();
    }
    return {};
}

//    delete from <database name>.<table name> where ...;
DB_RESULT_TYPE forDel(string s)
{
    string database;
    string table = "";
    SQLTree *condition = nullptr;
    smatch result;
    if (regex_search(s.cbegin(), s.cend(), result, regex("^\\s*delete\\s+from", regex::icase)))
    {
        s = remove(s, result[0].length());
    }
    else
    {
        throw runtime_error("invalid delete operation.");
    }
    vector<string> tempo = getDatabaseAndTable(s);
    database = tempo[0];
    table = tempo[1];
    s = tempo[2];
    if (regex_search(s.cbegin(), s.cend(), result, regex("^(\\s*where\\s+)(.+;)", regex::icase)))
    {
        s = remove(s, result[1].length());
        condition = getCondition(s);
        s = s.substr(result[2].length(), s.length());
    }
    else
    {
        throw runtime_error("invalid delete operation.");
    }
    cout << database << "  " << table << endl;
    if (condition != nullptr)
    {
        condition->display();
        cout << endl;
    }

    std::shared_ptr<SQLTree> _con(condition);
    Database _database(database);
    auto &_table = _database.table(table);
    if (condition != nullptr)
    {
        auto r = std::make_shared<TreeRecordChecker>(_con);
        return _table.erase(std::static_pointer_cast<RecordChecker>(r));
    }

    throw runtime_error("Can't delete record without where condition.");

    return {};
}

//    insert into table <database>.<table name>（<column_name>> []...）values（... ）;
DB_RESULT_TYPE forInsert(string s)
{
    string database;
    string table;
    vector<string> columns = vector<string>();
    vector<string> values;
    smatch result;
    regex_search(s.cbegin(), s.cend(), result, regex("^\\s*insert\\s+into\\s+table", regex::icase))
        ? s = remove(s, result[0].length())
        : throw runtime_error("invalid insert operation at beginning");
    if (regex_search(s.cbegin(), s.cend(), result, regex("^\\s*([\\w\\_]+?\\.[\\w\\_]+?)\\s*")))
    {
        vector<string> tempo = getDatabaseAndTable(s);
        database = tempo[0];
        table = tempo[1];
        s = tempo[2];
    }
    else
    {
        throw runtime_error("missing target table.");
    }
    if (regex_search(s.cbegin(), s.cend(), result, regex("^\\s*[(](.+?)[)]\\s*", regex::icase)))
    {
        columns = getline(result[1], item_type.at("column_name"), ",");
        s = remove(s, result[0].length());
    }
    if (regex_search(s.cbegin(), s.cend(), result, regex("^\\s*values\\s*[(](.+?)[)]\\s*", regex::icase)))
    {
        values = getline(result[1], item_type.at("value"), ",");
        s = remove(s, result[0].length());
    }
    else
    {
        throw runtime_error("require values as input.");
    }

    Database _database(database);
    auto &&_table = _database.table(table);

    std::vector<std::tuple<std::string, std::optional<std::string>>> _values;

    if (columns.size() != values.size())
    {
        throw std::invalid_argument("The numbers of columns and values doesn't match!");
    }

    for (int i = 0; i < columns.size(); i++)
    {
        std::cout << columns[i] << "  ";
        std::cout << values[i] << "  ";
        _values.emplace_back(columns[i], remove_quotes(values[i]));
    }

    _table.content().insert(_values);

    return {};
}

enum class update_state
{
    value,
    operation
};
//    update <database name>.<table name> set <column_name>=<value> where <某属性>=<某值>
DB_RESULT_TYPE forUpdate(string s)
{
    unordered_map<string, tuple<int, bool>> symble = {
        {",", {-1, false}}, {"where", {-1, false}}, {"=", {0, false}}, {"+", {1, true}},   {"-", {1, true}},
        {"*", {2, true}},   {"/", {2, true}},       {"(", {0, false}}, {")", {-1, false}}, {";", {-1, false}}};
    string database;
    string table;
    SQLTree *set_value = nullptr;
    SQLTree *condition = nullptr;
    smatch result;
    regex_search(s.cbegin(), s.cend(), result, regex("^\\s*Update", regex::icase))
        ? s = remove(s, result[0].length())
        : throw runtime_error("invalid update operation at beginning");
    if (regex_search(s.cbegin(), s.cend(), result, regex("^\\s*([\\w\\_]+?\\.[\\w\\_]+?)\\s*")))
    {
        vector<string> tempo = getDatabaseAndTable(s);
        database = tempo[0];
        table = tempo[1];
        s = tempo[2];
    }
    else
    {
        throw runtime_error("missing target table.");
    }
    //中间标志符
    if (regex_search(s.cbegin(), s.cend(), result, regex("^\\s*set\\s+", regex::icase)))
    {
        s = remove(s, result[0].length());
    }
    else
    {
        throw runtime_error("missing set");
    }
    vector<vector<string>> columns;
    vector<string> column;
    stack<SQLTree *> values;
    stack<string> operation;
    auto calc_step = [&]() {
        if (operation.empty())
            return;
        int prev = get<0>(symble.find(operation.top())->second);
        int cure = get<0>(symble.find(result[1])->second);
        if (prev > cure || operation.top() == result[1] || (prev == cure && get<1>(symble.find(result[1])->second)))
        {
            SQLTree *right_node = values.top();
            values.pop();
            SQLTree *left_node = values.top();
            values.pop();
            values.push(new SQLTree(operation.top(), left_node, right_node));
            operation.pop();
        }
    };
    bool flag = true;
    update_state state = update_state::value;
    while (flag)
    {
        switch (state)
        {
        case update_state::value:
            if (regex_search(s.cbegin(), s.cend(), result,
                             regex("^\\s*([\\w\\_]+|\\\'.+?\\\'|\\\".+?\\\")", regex::icase)))
            {
                values.push(new SQLTree(result[1]));
                s = remove(s, result[0].length());
                state = update_state::operation;
            }
            else if (regex_search(s.cbegin(), s.cend(), result, regex("^\\s*[(]", regex::icase)))
            {
                operation.push(result[1]);
                s = remove(s, result[0].length());
            }
            else
            {
                throw runtime_error("invalid value or column name.");
            }
            break;
        case update_state::operation:
            if (regex_search(s.cbegin(), s.cend(), result, regex("^\\s*(;)", regex::icase)))
            {
                while (operation.size() != 0)
                {
                    if (operation.top() == "(")
                    {
                        throw runtime_error("missing '('");
                    }
                    calc_step();
                }
                flag = false;
            }
            else if (regex_search(s.cbegin(), s.cend(), result, regex("^\\s*(,)", regex::icase)))
            {
                while (operation.size() != 0)
                {
                    if (operation.top() == "(")
                    {
                        throw runtime_error("missing '('");
                    }
                    calc_step();
                }
                s = remove(s, result[0].length());
                state = update_state::value;
            }
            else if (regex_search(s.cbegin(), s.cend(), result, regex("^\\s*(where)(.+?;)", regex::icase)))
            {
                while (operation.size() != 0)
                {
                    if (operation.top() == "(")
                    {
                        throw runtime_error("missing '('");
                    }
                    calc_step();
                }
                s = remove(s, result[0].length() - result[2].length());
                condition = getCondition(s);
                s = s.substr(result[2].length() - 1, s.length());
                flag = false;
            }
            else if (regex_search(s.cbegin(), s.cend(), result, regex("^\\s*([-+*/=\\(])", regex::icase)))
            {
                calc_step();
                operation.push(result[1]);
                state = update_state::value;
                s = remove(s, result[0].length());
            }
            else if (regex_search(s.cbegin(), s.cend(), result, regex("^\\s*([\\)])", regex::icase)))
            {
                while (operation.size() == 0 && operation.top() != "(")
                    calc_step();
                operation.size() == 0 ? throw runtime_error("missing '('") : operation.pop();
                operation.push(result[1]);
                s = remove(s, result[0].length());
            }
            else
            {
                throw runtime_error("invalid column description. unrecognizable column type");
            }
            break;
        default:
            throw runtime_error("parse function error");
        }
    }
    values.size() != 1 ? throw runtime_error("parse function error") : set_value = values.top();

    forEnd(s);
    std::cout << database << "  " << table << std::endl;
    if (set_value != nullptr)
        set_value->display();
    cout << endl;
    if (condition != nullptr)
        condition->display();
    cout << endl;
    return {};
}

//    select * from <database name>.<table name> join <> on condition where <某属性>=<某值>;
enum class select_status
{
    start,
    addion_table,
    condition,
};

DB_RESULT_TYPE forSelect(string s)
{
    vector<tuple<string, string>> columns = vector<tuple<string, string>>();
    vector<tuple<string, string, string, SQLTree *>> tables;
    SQLTree *condition = nullptr;
    smatch result;

    //获得列名
    if (regex_search(s.cbegin(), s.cend(), result, regex("^\\s*select\\s+", regex::icase)))
    {
        s = remove(s, result[0].length());
        if (regex_search(s.cbegin(), s.cend(), result, regex("^\\s*[*]\\s", regex::icase)))
        {
            columns.push_back({"*", "*"});
            s = remove(s, result[0].length());
        }
        else
        {
            while (true)
            {
                if (regex_search(s.cbegin(), s.cend(), result, regex("^\\s*([\\w]+)\\s*\\.\\s*([\\w]+)\\s*")))
                {
                    tuple<string, string> tempo = {result[1].str(), result[2].str()};
                    columns.push_back({result[1].str(), result[2].str()});
                    s = remove(s, result[0].length());
                }
                else if (regex_search(s.cbegin(), s.cend(), result, regex("^\\s*([\\w]+)\\s*")))
                {
                    columns.push_back({"*", result[1].str()});
                    s = remove(s, result[0].length());
                }
                else
                {
                    throw runtime_error("missing column name");
                }
                if (regex_search(s.cbegin(), s.cend(), result, regex("^\\s*,\\s*")))
                {
                    s = remove(s, result[0].length());
                }
                else if (regex_search(s.cbegin(), s.cend(), result, regex("^\\s*from\\s", regex::icase)))
                {
                    s = remove(s, result[0].length() - 5);
                    break;
                }
                else
                {
                    throw runtime_error("missing ',' or 'from' ");
                }
            }
        }
    }
    else
    {
        throw runtime_error("missing select column.");
    }
    //获得表
    select_status state = select_status::start;
    string database = "";
    string table = "";
    string alias = "";
    if (regex_search(s.cbegin(), s.cend(), result, regex("^\\s*from\\s+", regex::icase)))
    {
        s = remove(s, result[0].length());
        vector<string> tempo = getDatabaseAndTable(s);
        database = tempo[0];
        table = tempo[1];
        s = tempo[2];
        if (regex_search(s.cbegin(), s.cend(), result, regex("^\\s*([\\w]+?)(\\s|;)", regex::icase)))
        {
            string tempo = result[1];
            transform(tempo.begin(), tempo.end(), tempo.begin(), ::toupper);
            if (tempo != "JOIN" && tempo != "WHERE")
            {
                alias = result[1];
                s = remove(s, result[0].length() - 1);
            }
        }
        tables.push_back({database, table, alias, nullptr});
    }
    else
    {
        throw runtime_error("missing select table");
    }
    while (true)
    {
        if (regex_search(s.cbegin(), s.cend(), result,
                         regex("^(\\s*join\\s+).+?\\son\\s(.+?)(\\s+where\\s+|\\s+join\\s+|;)", regex::icase)))
        {
            int length = result[2].length();
            string condition = result[2];
            condition += ";";
            s = remove(s, result[1].length());
            vector<string> tempo = getDatabaseAndTable(s);
            database = tempo[0];
            table = tempo[1];
            s = tempo[2];
            if (regex_search(s.cbegin(), s.cend(), result, regex("^\\s*([\\w]+)??\\s+on\\s", regex::icase)))
            {
                if (result[1] != "")
                {
                    alias = result[1];
                }
                s = remove(s, result[0].length());
            }
            else
            {
                throw runtime_error("missing 'on'");
            }
            tables.push_back({database, table, alias, getCondition(condition)});
            s = s.substr(length, s.length());
        }
        else if (regex_search(s.cbegin(), s.cend(), result, regex("^(\\s*where\\s+)(.+?;)", regex::icase)))
        {
            s = remove(s, result[1].length());
            condition = getCondition(s);
            s = s.substr(result[2].length() - 1, s.length());
            break;
        }
        else if (regex_search(s.cbegin(), s.cend(), result, regex("^\\s*;", regex::icase)))
        {
            break;
        }
        else
        {
            throw runtime_error("mismatch table desc or select condition");
        }
    }
    forEnd(s);
    //    for (int i = 0; i < columns.size(); i++)
    //    {
    //        cout << "(" << get<0>(columns[i]) << "  " << get<1>(columns[i]) << ")";
    //    }
    //    cout << endl;
    //    for (int i = 0; i < tables.size(); i++)
    //    {
    //        cout << get<0>(tables[i]) << "  " << get<1>(tables[i]) << "  " << get<2>(tables[i]) << endl;
    //        if (get<3>(tables[i]) != nullptr)
    //            get<3>(tables[i])->display();
    //        cout << endl;
    //    }

    if (condition != nullptr)
        condition->display();
    cout << endl;

    std::shared_ptr<SQLTree> _con(condition);
    Database _database(get<0>(tables[0]));
    auto &_table = _database.table(get<1>(tables[0]));
    if (condition != nullptr)
    {
        auto r = std::make_shared<TreeRecordChecker>(_con);
        return _table.select(std::static_pointer_cast<RecordChecker>(r));
    }
    return _table.select();

    //    cout << endl;
    //    if (condition != nullptr)
    //        condition->display();
    //    cout << endl;
    return {};
}

string forEnd(std::string s)
{
    smatch result;
    //确认结束
    if (regex_search(s.cbegin(), s.cend(), result, regex("^\\s*;\\s*$")))
    {
        s = remove(s, result[0].length());
    }
    else if (regex_search(s.cbegin(), s.cend(), result, regex("\\s*;\\s*$")))
    {
        throw runtime_error("duplicate input before ';'.");
    }
    else if (regex_search(s.cbegin(), s.cend(), result, regex("^\\s*;\\s*")))
    {
        throw runtime_error("duplicate input after ';'.");
    }
    else
    {
        return s;
        throw runtime_error("missing ';' at the end of sql.");
    }
    return s;
}

vector<string> getDatabaseAndTable(string s)
{
    vector<string> final = vector<string>();
    smatch result;
    //获得数据库名
    if (regex_search(s.cbegin(), s.cend(), result, regex("^\\s*" + item_type.find("database")->second + "\\s*[.]")))
    {
        final.push_back(result[1]);
        s = remove(s, result[0].length());
    }
    else
    {
        throw runtime_error("unrognizable database name or missing '.'.");
    }
    //获得表名
    if (regex_search(s.cbegin(), s.cend(), result, regex("^\\s*" + item_type.find("table")->second + "\\s*")))
    {
        final.push_back(result[1]);
        s = remove(s, result[0].length());
    }
    else
    {
        throw runtime_error("table name unrecognizable");
    }
    final.push_back(s);
    return final;
}

enum class Column_State
{
    name,
    type,
    desc
};

std::vector<std::vector<std::string>> getColumns(std::string s)
{
    if (s.empty())
    {
        return vector<std::vector<std::string>>();
    }
    vector<vector<string>> columns;
    vector<string> column;
    Column_State state = Column_State::name;
    std::smatch result;

    while (s.size())
    {
        switch (state)
        {
        case Column_State::name:
            if (regex_search(s.cbegin(), s.cend(), result,
                             regex("^\\s*" + item_type.find("column_name")->second + "\\s", regex::icase)))
            {
                column = vector<string>();
                column.push_back(result[1]);
                s = remove(s, result[0].length());
                state = Column_State::type;
            }
            else
            {
                throw runtime_error("invalid column description. unrecognizable column name");
            }
            break;
        case Column_State::type:
            if (regex_search(s.cbegin(), s.cend(), result,
                             regex("^\\s*" + item_type.find("column_type")->second + "\\s*", regex::icase)))
            {
                column.push_back(result[1]);
                s = remove(s, result[0].length());
                state = Column_State::desc;
            }
            else
            {
                throw runtime_error("invalid column description. unrecognizable column type");
            }
            break;
        case Column_State::desc:
            if (regex_search(s.cbegin(), s.cend(), result, regex("^\\s*(primary|unique)\\s*", regex::icase)))
            {
                column.push_back(result[1]);
                s = remove(s, result[0].length());
            }
            else if (regex_search(s.cbegin(), s.cend(), result, regex("^\\s*(not)\\s+(null)\\s*", regex::icase)))
            {
                string tempo1 = result[1];
                string tempo2 = result[2];
                column.push_back(tempo1 + " " + tempo2);
                s = remove(s, result[0].length());
            }
            else if (regex_search(s.cbegin(), s.cend(), result,
                                  regex("^\\s*(default)\\s+" + item_type.find("value")->second + "\\s*", regex::icase)))
            {
                string tempo1 = result[1];
                string tempo2 = result[2];
                column.push_back(tempo1 + " " + tempo2);
                s = remove(s, result[0].length());
            }
            else if (regex_search(s.cbegin(), s.cend(), result, regex("^\\s*,\\s*", regex::icase)))
            {
                columns.push_back(column);
                s = remove(s, result[0].length());
                state = Column_State::name;
            }
            else
            {
                throw runtime_error("invalid column description. unrecognizable constrain as input.");
            }
            break;
        default:
            throw runtime_error("parse function error");
        }
    }
    if (state != Column_State::desc)
    {
        throw runtime_error("invalid column description. missing input at end.");
    }
    columns.push_back(column);
    return columns;
}

enum class condition_state
{
    strat,
    value,
    type,
    connection,
};

SQLTree *getCondition(string s)
{
    unordered_map<string, tuple<int, string>> symble = {
        {"begin", {0, "^\\s*([\\w\\_]+|\\\'.+\\\'|\\\".+\\\")"}},
        {">", {1, "^\\s*([\\w\\_]+|\\\'.+\\\'|\\\".+\\\")"}},
        {">=", {1, "^\\s*([\\w\\_]+|\\\'.+\\\'|\\\".+\\\")"}},
        {"<", {1, "^\\s*([\\w\\_]+|\\\'.+\\\'|\\\".+\\\")"}},
        {"<=", {1, "^\\s*([\\w\\_]+|\\\'.+\\\'|\\\".+\\\")"}},
        {"=", {1, "^\\s*([\\w\\_]+|\\\'.+\\\'|\\\".+\\\")"}},
        {"IN", {-1, "^\\s*[(](.+)[)]\\s*"}},
        {"LIKE", {1, "^\\s*(\\\".+\\\"|\\\'.+\\\')\\s*"}},
        {"BETWEEN", {2, "^\\s*([\\w\\_]+|[\\\'.+\\\']|\\\".+\\\")\\s+and\\s+([\\w\\_]+|[\\\'.+\\\']|\\\".+\\\")\\s*"}},
    };

    auto construct_and_or = [&](stack<SQLTree *> *storage, string symble) {
        if (storage->size() < 2)
        {
            throw runtime_error("mismatch storage length");
        }
        SQLTree *right_node = storage->top();
        storage->pop();
        SQLTree *left_node = storage->top();
        storage->pop();
        storage->push(new SQLTree(symble, left_node, right_node));
    };
    auto construct_not = [&](stack<SQLTree *> *storage, string symble) {
        if (storage->size() < 1)
        {
            throw runtime_error("mismatch storage length");
        }
        SQLTree *right_node = storage->top();
        storage->pop();
        storage->push(new SQLTree(symble, nullptr, right_node));
    };
    ;
    auto construct_tree = [&](stack<SQLTree *> *storage, stack<string> *connection_operator) {
        while (true)
        {
            if (connection_operator->size() == 0)
            {
                break;
            }
            if (connection_operator->top() == "AND" || connection_operator->top() == "OR")
            {
                construct_and_or(storage, connection_operator->top());
                connection_operator->pop();
            }
            else if (connection_operator->top() == "NOT")
            {
                construct_not(storage, connection_operator->top());
                connection_operator->pop();
            }
            else
            {
                break;
            }
        }
    };

    smatch result;
    condition_state state = condition_state::strat;
    stack<SQLTree *> storage = stack<SQLTree *>();
    stack<string> connection_operator = stack<string>();
    string type = "begin";
    while (s.size())
    {
        switch (state)
        {
        case condition_state::strat:
            if (regex_search(s.cbegin(), s.cend(), result, regex("^\\s*[(]", regex::icase)))
            {
                connection_operator.push("(");
                s = remove(s, result[0].length());
            }
            else if (regex_search(s.cbegin(), s.cend(), result, regex("^\\s*not", regex::icase)))
            {
                connection_operator.push("NOT");
                s = remove(s, result[0].length());
            }
            else
            {
                state = condition_state::value;
            }
            break;
        case condition_state::value: {
            tuple<int, string> value = symble.find(type)->second;
            switch (get<0>(value))
            {
            case 0:
                if (regex_search(s.cbegin(), s.cend(), result, regex(get<1>(value), regex::icase)))
                {
                    storage.push(new SQLTree(result[1]));
                    s = remove(s, result[0].length());
                    state = condition_state::type;
                }
                else
                {
                    throw runtime_error("invalid condition. column or value description unrecognizable");
                }
                break;
            case 1:
                if (regex_search(s.cbegin(), s.cend(), result, regex(get<1>(value), regex::icase)))
                {
                    storage.top()->set_right_node(new SQLTree(result[1]));
                    s = remove(s, result[0].length());
                    state = condition_state::connection;
                    construct_tree(&storage, &connection_operator);
                    type = "begin";
                }
                else
                {
                    throw runtime_error("invalid condition. column or value description unrecognizable after " + type);
                }
                break;
            case 2:
                if (regex_search(s.cbegin(), s.cend(), result, regex(get<1>(value), regex::icase)))
                {
                    storage.top()->set_right_node(new SQLTree("AND", new SQLTree(result[1]), new SQLTree(result[2])));
                    s = remove(s, result[0].length());
                    state = condition_state::connection;
                    construct_tree(&storage, &connection_operator);
                    type = "begin";
                }
                else
                {
                    throw runtime_error("invalid condition. mismatch for 'between'");
                }
                break;
            case -1:
                if (regex_search(s.cbegin(), s.cend(), result, regex(get<1>(value), regex::icase)))
                {
                    vector<string> tempo = getline(result[1], "[\\w\\_]+|\\\'.+\\\'|\\\".+\\\"", ",");
                    if (tempo.size() == 0)
                    {
                        throw runtime_error("invalid condition. can't be empty list");
                    }
                    SQLTree *node = new SQLTree(tempo[0]);
                    for (int i = 1; i < tempo.size(); i++)
                    {
                        node = new SQLTree("AND", new SQLTree(tempo[i]), node);
                    }
                    storage.top()->set_right_node(node);
                    s = remove(s, result[0].length());
                    state = condition_state::connection;
                    construct_tree(&storage, &connection_operator);
                    type = "begin";
                }
                else
                {
                    throw runtime_error("invalid condition. mismatch for 'in'");
                }
                break;
            default:
                throw runtime_error("function error. unhandle value type");
            }
            break;
        }
        case condition_state::type: {
            regex_search(s.cbegin(), s.cend(), result, regex("^\\s*(\\w+|[<>=]+)", regex::icase));
            type = result[1];
            transform(type.begin(), type.end(), type.begin(), ::toupper);
            if (symble.find(type) != symble.end())
            {
                s = remove(s, result[0].length());
                SQLTree *treenode = new SQLTree(type, storage.top(), nullptr);
                storage.pop();
                storage.push(treenode);
                state = condition_state::value;
            }
            else
            {
                throw runtime_error("invalid condition. Symble unrecognizable");
            }
            break;
        }
        case condition_state::connection:
            if (regex_search(s.cbegin(), s.cend(), result, regex("^\\s*;\\s*$", regex::icase)))
            {
                if (storage.size() == 1 && connection_operator.size() == 0)
                {
                    s = remove(s, result[0].length());
                }
                else
                {
                    throw runtime_error("finish too early");
                }
            }
            else if (regex_search(s.cbegin(), s.cend(), result, regex("^\\s*[)]\\s*", regex::icase)))
            {
                if (connection_operator.top() == "(")
                {
                    connection_operator.pop();
                    s = remove(s, result[0].length());
                    construct_tree(&storage, &connection_operator);
                }
                else
                {
                    throw runtime_error("missing '(' before ')'");
                }
            }
            else if (regex_search(s.cbegin(), s.cend(), result, regex("^\\s*(and|or)\\s", regex::icase)))
            {
                string tempo_connection = result[1];
                transform(tempo_connection.begin(), tempo_connection.end(), tempo_connection.begin(), ::toupper);
                connection_operator.push(tempo_connection);
                s = remove(s, result.length());
                state = condition_state::strat;
            }
            else
            {
                throw runtime_error("missing connection symble");
            }
            break;
        }
    }
    return storage.top();
}

enum class line_state
{
    value,
    seprator
};

std::vector<std::string> getline(string line, string type, string sep)
{
    line += sep;
    std::regex separator("^\\s*(" + type + ")\\s*" + sep);
    std::vector<std::string> final = std::vector<std::string>();
    std::smatch result;
    std::string::const_iterator iterStart = line.begin();
    std::string::const_iterator iterEnd = line.end();
    while (std::regex_search(iterStart, iterEnd, result, separator))
    {
        std::string column_desc = result[0];
        column_desc = column_desc.substr(0, column_desc.length() - 1);
        final.push_back(column_desc);
        iterStart = result[0].second;
    }
    return final;
}

}; // namespace parse

DB_RESULT_TYPE Parser::parse(std::string s)
{
    parse::string_length = 0;
    std::regex create("^\\s*create", regex::icase);
    std::regex alter("^\\s*alter", regex::icase);
    std::regex show("^\\s*show", regex::icase);
    std::regex drop("^\\s*drop", regex::icase);
    //    DROP database <database name>;
    std::regex del("^\\s*delete", regex::icase);
    std::regex insert("^\\s*insert", regex::icase);
    std::regex update("^\\s*update", regex::icase);
    std::regex select("^\\s*select", regex::icase);
    std::regex desc("\\s*desc", regex::icase);
    std::regex backup("^\\s*(backup|rollback)", regex::icase);
    //根节点操作，左节点数据库名，右节点是用户名
    std::regex authorization("^\\s*(revoke|grant)", regex::icase);
    //    grant <database name> to <user name>;
    //    revoke <database name> to <user name>;
    std::regex end(";", regex::icase);
    regex database("^\\s*(\\w+)", regex::icase);
    regex user("^\\s*(\\w+)", regex::icase);

    std::smatch result;
    std::string::const_iterator iterStart = s.begin();
    std::string::const_iterator iterEnd = s.end();
    if (regex_search(s.cbegin(), s.cend(), result, create))
    {
        return parse::forCreate(s);
    }
    else if (regex_search(s.cbegin(), s.cend(), result, alter))
    {
        return parse::forAlter(s);
    }
    else if (regex_search(s.cbegin(), s.cend(), result, show))
    {
        return parse::forShow(s);
    }
    else if (regex_search(s.cbegin(), s.cend(), result, drop))
    {
        return parse::forDrop(s);
    }
    else if (regex_search(s.cbegin(), s.cend(), result, del))
    {
        return parse::forDel(s);
    }
    else if (regex_search(s.cbegin(), s.cend(), result, insert))
    {
        return parse::forInsert(s);
    }
    else if (regex_search(s.cbegin(), s.cend(), result, update))
    {
        return parse::forUpdate(s);
    }
    else if (regex_search(s.cbegin(), s.cend(), result, select))
    {
        return parse::forSelect(s);
    }
    else if (regex_search(s.cbegin(), s.cend(), result, backup))
    {
        string operation = result[1];
        s = parse::remove(s, result[0].length());
        s = parse::forEnd(s);
        cout << operation << endl;
    }
    else if (regex_search(s.cbegin(), s.cend(), result, authorization))
    {
        return parse::forAuthorization(s);
    }
    else if (regex_search(s.cbegin(), s.cend(), result, desc))
    {
        return parse::forDesc(s);
    }
    else if (s == ";")
    {
        return {};
    }
    else
    {
        throw runtime_error("invalid sql input");
    }
    return {};
}