//
// Created by Chestnut on 2020/11/4.
//

#ifndef OHMYDBMS_USER_H
#define OHMYDBMS_USER_H

#include "../helper.h"

class UserList : public boost::serialization::singleton<UserList>
{
    std::unordered_map<std::string, std::string> _cache;

    friend class boost::serialization::detail::singleton_wrapper<UserList>;

    friend class boost::serialization::access;

    UserList();

  public:
    void validation(const std::string &username, const std::string &password);

    void delete_user(const std::string &username);

    void new_user(const std::string &username, const std::string &password);
};

#endif // OHMYDBMS_USER_H
