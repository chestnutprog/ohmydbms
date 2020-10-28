#include <iostream>

#include "database/database.h"
#include "parser/parser.h"
#include "state.h"

using namespace std;

int main(int argc, char *argv[])
{
    BOOST_LOG_TRIVIAL(info) << "Server started.";
    State state;
    Parser p;
    while (true)
    {
        string s, t;
        switch (state.session.session_status)
        {
        case Session::SessionStatus::LOGINED:
            cout << ">";
            do
            {
                cout << " ";
                cin >> t;
                s += t + " ";
            } while (t.find(';') == std::string::npos);
            try
            {
                cout << endl;
                auto [col_names, contents] = p.parse(s);
                for (auto &col_name : col_names)
                {
                    cout << setw(8) << col_name << ' ';
                }
                cout << endl;
                for (auto &row : contents)
                {
                    for (auto &entry : row)
                    {
                        if (entry.has_value())
                            cout << setw(8) << entry.value();
                        else
                            cout << setw(8) << "null";
                        cout << ' ';
                    }
                    cout << endl;
                }
            }
            catch (exception &e)
            {
                cout << e.what() << endl;
            }
            break;
        case Session::SessionStatus::INIT:
            cout << "Username:";
            cin >> s;
            cout << "Password:";
            cin >> t;
            try
            {
                state.session.login(s, t);
            }
            catch (exception &e)
            {
                cout << e.what() << endl;
            }
            break;
        default:
            throw runtime_error("Invalid state!");
        }
    }
    return 0;
}
