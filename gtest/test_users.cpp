//
// Created by Chestnut on 2020/11/18.
//
#include "../database/database.h"
#include "../sys/User.h"
#include "gtest/gtest.h"

TEST(test_users, validation)
{
    UserList::get_mutable_instance().validation("admin", "admin");
    UserList::get_mutable_instance().validation("admin", "admin");
    EXPECT_THROW({ UserList::get_mutable_instance().validation("admin", "wrong_password"); }, std::exception);
}