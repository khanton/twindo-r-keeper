#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/regex.hpp>
#include <sstream>
#include <iomanip>
#include <math.h>

#include "TwindoProto.h"
#include "HttpProto.h"

TwindoProto::TwindoProto(): config()
{
}

bool TwindoProto::init()
{
    namespace keywords = boost::log::keywords;
    namespace logging = boost::log;

    if (config.load()) {

        logging::trivial::severity_level logSeverity;

        std::string level(config.log_level);
        std::istringstream{ level } >> logSeverity;

        logging::add_file_log(
            keywords::file_name = config.log_path,
            keywords::auto_flush = true,
            keywords::format = "[%TimeStamp%] (%Severity%): %Message%",
            keywords::open_mode = std::ios_base::app);

        logging::add_common_attributes();

        logging::core::get()->set_filter(
            logging::trivial::severity >= logSeverity
        );

    }

    boost::regex mask("^\\d{6}$$");
    boost::cmatch what;

    if (!boost::regex_match(config.place.c_str(), what, mask)) {
        BOOST_LOG_TRIVIAL(error) << "Twindo libary fail! Invalid place code format. It must be 6 digit.";
        return false;
    }

    BOOST_LOG_TRIVIAL(info) << "Twindo library started with code=" << config.place;
 
    return true;
}

bool TwindoProto::setUser(const uint64_t card, std::string& _user) {

    std::ostringstream ss;
    ss << std::setfill('0') << std::setw(8) << card;

    user = ss.str();

    BOOST_LOG_TRIVIAL(trace) << "Card request. user=" << user;

    boost::regex mask("^\\d{8}$$");
    boost::cmatch what;

    if (!boost::regex_match(user.c_str(), what, mask)) {
        BOOST_LOG_TRIVIAL(trace) << "Card request. Twindo can't process this card number";
        return false;
    }

    _user = user;

    return true;
}

bool TwindoProto::get(const std::string& user, std::string& name)
{
    HttpProto api(HttpProto::utf8_to_wstring(config.api_path));

    std::string url("/places/cashdesk/");
    url += config.place + "/" + user;

    std::string response;

    BOOST_LOG_TRIVIAL(info) << "Card request. url='" << config.api_path << url << "', user=" << user << ", place=" << config.place;

    DWORD rcode = api.get(HttpProto::utf8_to_wstring(url), response);

    if (rcode < 200 || rcode > 299) {
        BOOST_LOG_TRIVIAL(error) << "Card request error! code=" << rcode;
        return false;
    }

    BOOST_LOG_TRIVIAL(trace) << "Card request success!";

    try {
        boost::property_tree::ptree pt;
        std::stringstream resp(response);
        boost::property_tree::json_parser::read_json(resp, pt);

        std::string user_name = pt.get<std::string>("name");
        std::string currency = pt.get<std::string>("currency");
        // TODO: Check currency for multicyrrency system
   
        if (!user_name.empty()) {
            BOOST_LOG_TRIVIAL(trace) << "Card request return name='" << user_name <<"'";
            name = user_name;
            return true;
        }
    }
    catch (const boost::property_tree::json_parser::json_parser_error & error) {
        BOOST_LOG_TRIVIAL(error) << "Response parse error='" << error.what() << "', json='" << response << "'";
    }

    return false;
}

bool TwindoProto::balance(const std::string& user, const double summa, unsigned long& balance, unsigned long& allowed, unsigned long& allowed_in_currency)
{
    HttpProto api(HttpProto::utf8_to_wstring(config.api_path));

    std::string url("/places/cashdesk/");
    url += config.place + "/" + user;

    std::string response;

    BOOST_LOG_TRIVIAL(info) << "Card balance. url='" << config.api_path << url << "', user=" << user << ", place=" << config.place;

    std::string buf;

    std::ostringstream ostr(buf);
    ostr << "{ \"ordersum\": " << std::fixed << std::setprecision(0) << summa << "}";

    api.addHeader(L"Content-Type", L"application/json");
    
    DWORD rcode = api.post(HttpProto::utf8_to_wstring(url), ostr.str(), response);

    if (rcode < 200 || rcode > 299) {
        BOOST_LOG_TRIVIAL(error) << "Card response error! code=" << rcode;
        return false;
    }

    try {
        boost::property_tree::ptree pt;
        std::stringstream resp(response);
        boost::property_tree::json_parser::read_json(resp, pt);

        balance = pt.get<unsigned long>("balance");
        allowed = pt.get<unsigned long>("allowed");
        allowed_in_currency = pt.get<unsigned long>("allowed_in_currency");
        
        BOOST_LOG_TRIVIAL(info) << "Card request success! user=" << user 
                << ", check summa=" << summa 
                << ", balance=" << balance 
                << ", allowed=" << allowed
                << ", allowed_in_currency=" << allowed_in_currency
            ;

        return true;
    }
    catch (const boost::property_tree::json_parser::json_parser_error & error) {
        BOOST_LOG_TRIVIAL(error) << "Response parse error='" << error.what() << "', json='" << response << "'";
    }

    return false;
}

bool TwindoProto::commit(const std::string& user, const double summa, const double writeoff_sum, const std::string &info)
{
    HttpProto api(HttpProto::utf8_to_wstring(config.api_path));

    std::string url("/places/cashdesk/");
    url += config.place + "/" + user;

    std::string response;

    BOOST_LOG_TRIVIAL(info) << "Card commit url='" << config.api_path << url 
        << "', user=" << user 
        << ", place=" << config.place 
        << ", ordersum=" << summa
        << ", writeof_sum=" << summa
        ;

    std::ostringstream ostr;
    ostr << "{ \"ordersum\": " << std::fixed << std::setprecision(0) << summa 
                << ", \"writeoff_sum\":" << writeoff_sum 
                << ", \"cashdesk_info\":" << info 
        << " }";

    api.addHeader(L"Content-Type", L"application/json");

    DWORD rcode = api.post(HttpProto::utf8_to_wstring(url), ostr.str(), response);

    if (rcode < 200 || rcode > 299) {
        BOOST_LOG_TRIVIAL(error) << "Card response error! code=" << rcode;
        return false;
    }

    try {
        boost::property_tree::ptree pt;
        std::stringstream resp(response);
        boost::property_tree::json_parser::read_json(resp, pt);

        std::string ret = pt.get("status","error");
        unsigned int count = pt.get<unsigned int>("count", 0);

        if (ret == "OK") {

            BOOST_LOG_TRIVIAL(info) << "Commit success! user=" << user << ", count=" << count;

            return true;
        }

        BOOST_LOG_TRIVIAL(error) << "Commit error! json='" << response << "'";
    }
    catch (const boost::property_tree::json_parser::json_parser_error & error) {
        BOOST_LOG_TRIVIAL(error) << "Response parse error='" << error.what() << "', json='" << response << "'";
    }

	return false;
}

void TwindoProto::trace(const std::string& trace)
{
    BOOST_LOG_TRIVIAL(error) << trace;
}

void TwindoProto::error(const std::string& error)
{
    BOOST_LOG_TRIVIAL(error) << "Farcars report error:" << error;
}
