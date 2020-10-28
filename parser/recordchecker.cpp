//
// Created by Chestnut on 2020/11/21.
//

#include "recordchecker.h"
#include "../database/record.h"
#include "../helper.h"

TreeRecordChecker::TreeRecordChecker(std::shared_ptr<SQLTree> condition) : _condition(condition)
{
}

bool TreeRecordChecker::check(Record &record)
{
    if (!_condition)
        return true;

    std::function<bool(SQLTree *)> run = [&](SQLTree *tree) -> bool {
        if (!tree->left_node->left_node)
        {

            if (tree->context == "=")
            {
                return record.entry(tree->left_node->context)->operator==(tree->right_node->context);
            }
            else if (tree->context == ">")
            {
                return true;
            }
            else if (tree->context == "<")
            {
                return true;
            }
        }
        else
        {
            if (tree->context == "AND")
            {
                return run(tree->left_node) && run(tree->right_node);
            }
            else if (tree->context == "OR")
            {
                return run(tree->left_node) || run(tree->right_node);
            }
            else if (tree->context == "NOT")
            {
                return !run(tree->right_node);
            }
        }
        return true;
    };

    return run(_condition.get());
}

SimpleRecordChecker::SimpleRecordChecker(std::string col_name, std::string value) : col_name(col_name), value(value)
{
}

bool SimpleRecordChecker::check(Record &record)
{
    return record.entry(col_name)->operator==(value);
}
