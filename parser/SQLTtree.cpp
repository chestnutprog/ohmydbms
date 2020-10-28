//
// Created by xuzih on 2020/11/11.
//

#include "SQLTtree.h"

SQLTree::SQLTree(std::string context) : context(context), left_node(nullptr), right_node(nullptr)
{
}

SQLTree::SQLTree(std::string context, SQLTree *leftNode, SQLTree *rightNode)
    : context(context), left_node(leftNode), right_node(rightNode)
{
}

void SQLTree::display()
{
    std::cout << context << "  ";
    if (left_node != nullptr)
    {
        left_node->display();
    }
    if (right_node != nullptr)
    {
        right_node->display();
    }
}
