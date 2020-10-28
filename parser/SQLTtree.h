//
// Created by xuzih on 2020/11/11.
//

#ifndef TEST_SQLTTREE_H
#define TEST_SQLTTREE_H

#include <iostream>
#include <string>

class SQLTree
{
    friend class TreeRecordChecker;
  public:
    SQLTree(std::string context);
    SQLTree(std::string context, SQLTree *leftNode, SQLTree *rightNode);
    std::string get_context()
    {
        return context;
    };
    void set_context(std::string context)
    {
        this->context = context;
    };
    void set_left_node(SQLTree *node)
    {
        this->left_node = node;
    };
    void set_right_node(SQLTree *node)
    {
        this->right_node = node;
    };
    void display();

  private:
    std::string context;
    SQLTree *left_node;
    SQLTree *right_node;
};

#endif // TEST_SQLTTREE_H
