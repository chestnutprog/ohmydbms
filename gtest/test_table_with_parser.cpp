//
// Created by Chestnut on 2020/11/24.
//
#include "../database/database.h"
#include "../parser/parser.h"
#include "gtest/gtest.h"

#include "test.h"

TEST(test_table_parser, select)
{
    Parser p;
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
        table.create({{"Username", {FieldType::VARCHAR, 20}},
                      {"password", {FieldType::VARCHAR, 10}},
                      {"test", {FieldType::INTEGER, {}}}});

        TEST_COUT << "Inserting some rows";
        table.content().insert({{"Username", "admin"}, {"password", "admin"}, {"test", "114514"}});
        table.content().insert({{"Username", "root"}, {"password", "rootroot"}, {"test", "114514"}});
        table.content().insert({{"Username", "qwq"}});
    }
    {
        auto [col_names, contents] = p.parse("select * from test_database.test_table where Username = admin;");
        EXPECT_EQ(contents.size(), 1);
    }
    {
        auto [col_names, contents] = p.parse("select * from test_database.test_table where Username = root;");
        EXPECT_EQ(contents.size(), 1);
    }
    {
        auto [col_names, contents] = p.parse("select * from test_database.test_table where Username = rootroot;");
        EXPECT_EQ(contents.size(), 0);
    }
    {
        auto [col_names, contents] = p.parse("select * from test_database.test_table where Username = adm123in;");
        EXPECT_EQ(contents.size(), 0);
    }
    {
        Database database("test_database");
        database.del();
    }
}

TEST(test_table_parser, insert)
{
    Parser p;
    {
        TEST_COUT << "Cleaning old data";
        Database database("test_database3");
        database.del_if_exist();
        TEST_COUT << "Creating database";
        database.create();
    }
    {
        Database database("test_database3");
        auto &&table = database.table("test_table");
        table.create({{"Username", {FieldType::VARCHAR, 20}},
                      {"password", {FieldType::VARCHAR, 10}},
                      {"test", {FieldType::INTEGER, {}}}});

        TEST_COUT << "Inserting some rows";
        table.content().insert({{"Username", "admin"}, {"password", "admin"}, {"test", "114514"}});
        table.content().insert({{"Username", "root"}, {"password", "rootroot"}, {"test", "114514"}});
        table.content().insert({{"Username", "qwq"}});
    }
    {
        Database database("test_database3");
        auto &&table = database.table("test_table2");
        table.create({{"Username", {FieldType::VARCHAR, 20}},
                      {"password", {FieldType::VARCHAR, 10}},
                      {"test", {FieldType::INTEGER, {}}}});

        TEST_COUT << "Inserting some rows";
        table.content().insert({{"Username", "admin"}, {"password", "admin"}, {"test", "114514"}});
        table.content().insert({{"Username", "root"}, {"password", "rootroot"}, {"test", "114514"}});
        table.content().insert({{"Username", "qwq"}});
    }
}