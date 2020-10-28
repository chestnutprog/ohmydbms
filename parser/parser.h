//
// Created by Chestnut on 2020/11/4.
//

#ifndef OHMYDBMS_PARSER_H
#define OHMYDBMS_PARSER_H

#include "../helper.h"
#include "SQLTtree.h"
#include <regex>
#include <string>
#include <vector>

using namespace std;
class Parser
{
  public:
    DB_RESULT_TYPE parse(std::string query);
};

#endif // OHMYDBMS_PARSER_H
