#include "helper.h"
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <json_dto/pub.hpp>
#include <restinio/all.hpp>

#include "parser/parser.h"

using boost::property_tree::ptree;
using boost::property_tree::read_json;
using boost::property_tree::write_json;

using namespace restinio;
using namespace std;

struct sql_t
{
    std::string sql;
    template <typename Json_Io> void json_io(Json_Io &io)
    {
        io &json_dto::mandatory("sql", sql);
    }
};

struct data_t
{
    vector<string> *cols;
    vector<vector<optional<string>>> *rows;
    template <typename Json_Io> void json_io(Json_Io &io)
    {
        io &json_dto::mandatory("cols", *cols) & json_dto::mandatory("rows", *rows);
    }
};

struct message_t
{
    std::string status;
    data_t data;

    // Entry point for json_dto.
    template <typename Json_Io> void json_io(Json_Io &io)
    {
        io &json_dto::mandatory("status", status) & json_dto::mandatory("result", data);
    }
};

struct message_error_t
{
    std::string status;
    std::string error;

    // Entry point for json_dto.
    template <typename Json_Io> void json_io(Json_Io &io)
    {
        io &json_dto::mandatory("status", status) & json_dto::mandatory("error", error);
    }
};

int main(int argc, char *argv[])
{
    BOOST_LOG_TRIVIAL(info) << "Server started.";
    auto router = std::make_unique<router::express_router_t<>>();

    router->http_get(R"(/)", [](auto req, auto params) {
        BOOST_LOG_TRIVIAL(info) << "HTTP GET Request on /.";
        return req->create_response().set_body("Welcome to my DBMS.").done();
    });

    router->http_post(R"(/)", [](auto req, auto params) {
        BOOST_LOG_TRIVIAL(info) << fmt::format("HTTP POST Request on /. Body:{} ", req->body());
        auto &&response =
            req->create_response(); //.append_header(restinio::http_field::content_type, "text/json; charset=utf-8");

        if (!req->header().has_field("username") || !req->header().has_field("password"))
        {
            message_error_t message;
            message.status = "error";
            message.error = "Invaild request: Username or password Header missing.";
            std::string _json = json_dto::to_json(message);
            BOOST_LOG_TRIVIAL(debug) << "Result(error): " << _json;
            return response.set_body(_json).done();
        }

        Parser p;
        try
        {
            message_t message;
            message.status = "success";
            sql_t sql;
            json_dto::from_json(req->body(),sql);
            auto result = p.parse(sql.sql);
            message.data.cols = &result.first;
            message.data.rows = &result.second;
            std::string _json = json_dto::to_json(message);
            BOOST_LOG_TRIVIAL(debug) << "Result: " << _json;
            return response.set_body(_json).done();
        }
        catch (std::exception &e)
        {
            message_error_t message;
            message.status = "error";
            message.error = e.what();
            std::string _json = json_dto::to_json(message);
            BOOST_LOG_TRIVIAL(info) << "Result(error): " << _json;
            return response.set_body(_json).done();
        }

        return response.set_body("Welcome to my DBMS.").done();
    });

    router->non_matched_request_handler(
        [](auto req) { return req->create_response(restinio::status_not_found()).connection_close().done(); });

    // Launching a server with custom traits.
    struct my_server_traits : public default_single_thread_traits_t
    {
        using request_handler_t = restinio::router::express_router_t<>;
    };
    BOOST_LOG_TRIVIAL(info) << "Start HTTP Server on port 8080.";
    restinio::run(
        restinio::on_this_thread<my_server_traits>().port(8080).address("0.0.0.0").request_handler(std::move(router)));
}
