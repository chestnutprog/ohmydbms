//
// Created by Chestnut on 2020/11/4.
//

#include "Session.h"
#include "sys/User.h"

#include <system_error>

void Session::login(std::string username, std::string password)
{
    UserList::get_mutable_instance().validation(username, password);
    session_status = SessionStatus::LOGINED;
}

void Session::logout()
{
    session_status = SessionStatus::INIT;
}