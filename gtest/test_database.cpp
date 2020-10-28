//
// Created by Chestnut on 2020/11/9.
//
#include "../database/database.h"
#include "gtest/gtest.h"

#include "test.h"

TEST(test_database, create_database)
{
    {
        TEST_COUT << "Cleaning old data";
        Database database("test_database");
        database.del_if_exist();
        TEST_COUT << "Creating database";
        database.create();
    }
    {
        TEST_COUT << "Create the same database twice";
        Database database("test_database");
        EXPECT_THROW({ database.create(); }, std::exception);
    }
    {
        TEST_COUT << "Drop the same database twice";
        Database database("test_database");
        database.del_if_exist();
        EXPECT_THROW({ database.del(); }, std::exception);
    }
}

TEST(test_database, create_table)
{
    {
        TEST_COUT << "Cleaning old data";
        Database database("test_database");
        database.del_if_exist();
        TEST_COUT << "Creating database";
        database.create();

        auto &&table = database.table("test_table");
        TEST_COUT << "Creating empty table";
        table.create({});
    }
    {
        Database database("test_database");
        auto &&table = database.table("test_table");
        TEST_COUT << "Create the same table twice";
        EXPECT_THROW({ table.create({}); }, std::exception);
        TEST_COUT << "Drop the same database twice";
        table.del();
        EXPECT_THROW({ table.del(); }, std::exception);
    }
    {
        Database database("test_database");
        database.del();
    }
}

TEST(test_database, create_table_struct)
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
        TEST_COUT << "Creating table including varchar without length";
        EXPECT_THROW({ table.create({{"test", {FieldType::VARCHAR, {}}}}); }, std::exception);

        TEST_COUT << "Creating table including multiple same col";
        EXPECT_THROW(
            {
                table.create({{"test", {FieldType::VARCHAR, 20}},
                              {"test", {FieldType::VARCHAR, 10}},
                              {"test", {FieldType::VARCHAR, 2}}});
            },
            std::exception);

        TEST_COUT << "Creating a normal table";
        table.create({{"username", {FieldType::VARCHAR, 20}},
                      {"password", {FieldType::VARCHAR, 10}},
                      {"test", {FieldType::INTEGER, 2}}});
        EXPECT_TRUE(table.exist());

        TEST_COUT << "Drop table";
        table.del();

        TEST_COUT << "Recreating table";
        table.create({{"username", {FieldType::VARCHAR, 20}},
                      {"password", {FieldType::VARCHAR, 10}},
                      {"test", {FieldType::INTEGER, 2}}});
    }
    {
        Database database("test_database");
        database.del();
    }
}
