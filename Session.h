//
// Created by Chestnut on 2020/11/4.
//

#ifndef OHMYDBMS_SESSION_H
#define OHMYDBMS_SESSION_H

#include <string>

class Session
{
  private:
  public:
    enum class SessionStatus
    {
        INIT,
        LOGINED
    };
    SessionStatus session_status;

    Session() : session_status(SessionStatus::INIT)
    {
    }

    void login(std::string username, std::string password);

    void logout();
};

#endif // OHMYDBMS_SESSION_H
