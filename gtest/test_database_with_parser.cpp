//
// Created by Chestnut on 2020/11/18.
//
#include "../database/database.h"
#include "../parser/parser.h"
#include "gtest/gtest.h"

TEST(test_parser, create_drop_database)
{
    Parser p;
    {
        Database("test_database").del_if_exist();
        Database("test_database2").del_if_exist();
        p.parse("create database test_database;");
        p.parse("Create dataBase test_database2;");
        EXPECT_THROW({ p.parse("Create dataBase test_database2;"); }, std::exception);
    }
    {
        Database database("test_database");
        database.del();
        Database("test_database2").del();
    }
}

TEST(test_parser, create_drop_table)
{
    Parser p;
    {
        Database("test_database").del_if_exist();
        Database("test_database2").del_if_exist();
        p.parse("create database test_database;");
        p.parse("Create dataBase test_database2;");
        EXPECT_THROW({ p.parse("Create dataBase test_database2;"); }, std::exception);
    }
    {
        p.parse("Create Table test_table to test_database");
        //        p.parse("Create Table test_table2 to test_database(a int primary,b bool not null,c double unique
        //        primary,d "
        //                "varchar(2) default \"123\",e datetime unique not null default 123);");
    }
    {
        p.parse("Drop table test_database.test_table;");
        EXPECT_THROW({ p.parse("Drop table test_database.test_table;"); }, std::exception);
        p.parse("Drop database test_database;");
        EXPECT_THROW({ p.parse("Drop database test_database;"); }, std::exception);
    }
}

TEST(test_parser, show_database)
{
    Parser p;
    {
        Database("test_database").del_if_exist();
        Database("test_database2").del_if_exist();
    }
    {
        auto [col_names1, result1] = p.parse("show database;");
        auto before_c = result1.size();
        p.parse("create database test_database;");
        p.parse("Create dataBase test_database2;");
        auto [col_names2, result2] = p.parse("show database;");
        EXPECT_EQ(result2.size(), before_c + 2);
    }
    {
        Database database("test_database");
        database.del();
        Database("test_database2").del();
    }
}

TEST(test_parser, show_tables)
{
    Parser p;
    {
        Database("test_database").del_if_exist();
        Database("test_database2").del_if_exist();
        p.parse("create database test_database;");
        p.parse("Create dataBase test_database2;");
    }
    {
        Database database("test_database");
        auto &&table = database.table("test_table");
        table.create({{"Username", {FieldType::VARCHAR, 20}},
                      {"password", {FieldType::VARCHAR, 10}},
                      {"test", {FieldType::INTEGER, {}}}});

        table.content().insert({{"Username", "admin"}});
        p.parse("show tables in test_database;");
        p.parse("show tables in test_database2;");
        EXPECT_THROW({ p.parse("show tables in ___test_database3;"); }, std::exception);
    }
    {
        Database database("test_database");
        auto &&table = database.table("test_table2");
        table.create({{"username", {FieldType::VARCHAR, 20}},
                      {"password", {FieldType::VARCHAR, 10}},
                      {"test", {FieldType::INTEGER, {}, true}}});
        EXPECT_THROW({ table.content().insert({{"username", "admin"}}); }, std::exception);
        auto [col_names, result] = p.parse("show tables in test_database;");
        EXPECT_EQ(result.size(), 2);
    }
    {
        Database database("test_database");
        database.del();
        Database("test_database2").del();
    }
}

TEST(test_parser, desc_tables)
{
    Parser p;
    {
        Database("test_database").del_if_exist();
        Database("test_database2").del_if_exist();
        p.parse("create database test_database;");
        p.parse("Create dataBase test_database2;");
    }
    {
        Database database("test_database");
        auto &&table = database.table("test_table");
        table.create({{"Username", {FieldType::VARCHAR, 20}},
                      {"password", {FieldType::VARCHAR, 10}},
                      {"test", {FieldType::INTEGER, {}}}});

        table.content().insert({{"Username", "admin"}});
        p.parse("desc test_database.test_table;");
        EXPECT_THROW({ p.parse("desc test_database2.test_table;"); }, std::exception);
    }
    {
        Database database("test_database");
        auto &&table = database.table("test_table2");
        table.create({{"username", {FieldType::VARCHAR, 20}},
                      {"password", {FieldType::VARCHAR, 10}},
                      {"test", {FieldType::INTEGER, {}, true}}});
        p.parse("desc test_database.test_table2;");
    }
    {
        Database database("test_database");
        database.del();
        Database("test_database2").del();
    }
}