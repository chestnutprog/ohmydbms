//
// Created by Chestnut on 2020/11/4.
//

#include "User.h"
#include "../database/database.h"

UserList::UserList()
{
    Database database("sys");
    if (!database.exist())
    {
        database.create();
    }
    auto &user_table = database.table("users");
    if (!user_table.exist())
    {
        user_table.create({
            {"username", {FieldType::VARCHAR, 20, true, true}},
            {"password", {FieldType::VARCHAR, 10, true}},
        });
        user_table.content().insert({{"username", "admin"}, {"password", "admin"}});
    }
}

void UserList::validation(const std::string &username, const std::string &password)
{
    if (_cache.count(username))
    {
        if (_cache.at(username) == password)
            return;
    }
    Database database("sys");
    auto &user_table = database.table("users");
    auto [col_names, users] = user_table.select();
    auto username_id = find(col_names.begin(), col_names.end(), "username") - col_names.begin();
    auto password_id = find(col_names.begin(), col_names.end(), "password") - col_names.begin();
    for (auto &user : users)
    {
        if (user[username_id] == username && user[password_id] == password)
        {
            _cache[username] = password;
            return;
        }
    }
    throw std::invalid_argument("Username or password incorrect!");
}

void UserList::new_user(const std::string &username, const std::string &password)
{
    Database database("sys");
    auto &user_table = database.table("users");
    user_table.content().insert({{"username", username}, {"password", password}});
}

void UserList::delete_user(const std::string &username)
{
    std::shared_ptr<RecordChecker> checker = std::make_shared<SimpleRecordChecker>("username", username);
    Database database("sys");
    auto &user_table = database.table("users");
    user_table.erase(checker);
}