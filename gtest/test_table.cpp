//
// Created by Chestnut on 2020/11/24.
//
#include "../database/database.h"
#include "gtest/gtest.h"

#include "test.h"

TEST(test_table, insert)
{
    {
        TEST_COUT << "Cleaning old data";
        Database database("test_database");
        database.del_if_exist();
        TEST_COUT << "Creating database";
        database.create();
    }
    {
        Database database("test_database");
        auto &&table = database.table("test_table");

        TEST_COUT << "Creating a normal table";
        table.create({{"Username", {FieldType::VARCHAR, 20}},
                      {"password", {FieldType::VARCHAR, 10}},
                      {"test", {FieldType::INTEGER, {}}}});

        TEST_COUT << "Insert a row";
        table.content().insert({{"Username", "admin"}});
    }
    {
        Database database("test_database");
        auto &&table = database.table("test_table2");
        table.create({{"username", {FieldType::VARCHAR, 20, true, true}},
                      {"password", {FieldType::VARCHAR, 10}},
                      {"test", {FieldType::INTEGER, {}, true, true}}});
        EXPECT_THROW({ table.content().insert({{"username", "admin"}}); }, std::exception);
        table.content().insert({{"username", "admin"}, {"test", "123"}});
        EXPECT_THROW({ table.content().insert({{"username", "admin"}, {"test", "123"}}); }, std::exception);
        EXPECT_THROW({ table.content().insert({{"username", "admin2"}, {"test", "123"}}); }, std::exception);
    }
    {
        Database database("test_database");
        auto &&table = database.table("test_table3");
        table.create({{"username", {FieldType::VARCHAR, 20, true, true}},
                      {"password", {FieldType::VARCHAR, 10}},
                      {"test", {FieldType::INTEGER, {}, false, true}}});
        table.content().insert({{"username", "admin"}});
        table.content().insert({{"username", "admin2"}, {"test", "123"}});
        EXPECT_THROW({ table.content().insert({{"username", "admin"}, {"test", "123"}}); }, std::exception);
        EXPECT_THROW({ table.content().insert({{"username", "admin3"}, {"test", "123"}}); }, std::exception);
    }
    {
        Database database("test_database");
        database.del();
    }
}

TEST(test_table, select)
{
    {
        Database database("test_database");
        database.del_if_exist();
        database.create();
    }
    {
        Database database("test_database");
        auto &&table = database.table("test_table");
        table.create({{"Username", {FieldType::VARCHAR, 20}},
                      {"password", {FieldType::VARCHAR, 10}},
                      {"test", {FieldType::INTEGER, {}}}});

        table.content().insert({{"Username", "admin"}, {"password", "admin"}, {"test", "114514"}});
        table.content().insert({{"Username", "root"}, {"password", "rootroot"}, {"test", "114514"}});
        table.content().insert({{"Username", "qwq"}});
    }
    {
        Database database("test_database");
        auto &&table = database.table("test_table");
        auto result = table.select();
        auto &[col_names, contents] = result;
    }
    {
        Database database("test_database");
        database.del();
    }
}

TEST(test_table, alter_table)
{
    {
        TEST_COUT << "Cleaning old data";
        Database database("test_database");
        database.del_if_exist();
        TEST_COUT << "Creating database";
        database.create();
    }
    {
        Database database("test_database");
        auto &&table = database.table("test_table");

        TEST_COUT << "Creating a normal table";
        table.create({{"Username", {FieldType::VARCHAR, 20}},
                      {"password", {FieldType::VARCHAR, 10}},
                      {"test", {FieldType::INTEGER, {}}}});

        TEST_COUT << "Inserting some rows";
        table.content().insert({{"Username", "admin"}, {"password", "admin"}, {"test", "114514"}});
        table.content().insert({{"Username", "root"}, {"password", "rootroot"}, {"test", "114514"}});
        table.content().insert({{"Username", "qwq"}});

        TEST_COUT << "Rename col names wrongly";
        EXPECT_THROW({ table.rename_col("Username", "Username"); }, std::exception);
        EXPECT_THROW({ table.rename_col("Username", "test"); }, std::exception);

        TEST_COUT << "Rename col names";
        table.rename_col("Username", "test2");
    }
    {
        Database database("test_database");
        auto &&table = database.table("test_table");
        auto result = table.select();
        auto &[col_names, contents] = result;
        TEST_COUT << "Checking rename result";
        EXPECT_TRUE(std::find(col_names.begin(), col_names.end(), "test2") != col_names.end());
    }
    {
        Database database("test_database");
        database.del();
    }
}