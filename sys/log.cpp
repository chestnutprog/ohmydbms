//
// Created by Chestnut on 2020/11/19.
//

#include "../helper.h"

#include "log.h"

#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/file.hpp>

const fs::path logpath = basepath / "log";

class InitDirectories
{
  public:
    InitDirectories()
    {
        fs::create_directories(datapath);
        fs::create_directories(logpath);
    }
};

class InitLogfile
{
  public:
    InitLogfile()
    {
        const fs::path logfile = logpath / "server.log";
        namespace keywords = boost::log::keywords;
        boost::log::add_common_attributes();
        boost::log::add_file_log(keywords::file_name = logfile, keywords::open_mode = std::ios_base::app,
                                 keywords::auto_flush = true,
                                 keywords::format = "[%TimeStamp%] (%ProcessID%:%ThreadID%) <%Severity%>: %Message%");
        boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::info);

        BOOST_LOG_TRIVIAL(info) << "Log module inited.";
        //        BOOST_LOG_TRIVIAL(trace) << "A trace severity message";
        //        BOOST_LOG_TRIVIAL(debug) << "A debug severity message";
        //        BOOST_LOG_TRIVIAL(info) << "An informational severity message";
        //        BOOST_LOG_TRIVIAL(warning) << "A warning severity message";
        //        BOOST_LOG_TRIVIAL(error) << "An error severity message";
        //        BOOST_LOG_TRIVIAL(fatal) << "A fatal severity message";
    }
};

static InitDirectories _directories_init;

static InitLogfile _logfile_init;

DB_RESULT_TYPE Logger::show_log()
{
    std::vector<std::string> col_names = {"log"};
    std::vector<std::vector<std::optional<std::string>>> content;
    std::ifstream lf(logpath / "server.log");
    std::string s;
    while (std::getline(lf, s))
    {
        std::vector<std::optional<std::string>> row;
        row.emplace_back(std::move(s));
        content.emplace_back(std::move(row));
    }

    return make_pair(std::move(col_names), std::move(content));
}
