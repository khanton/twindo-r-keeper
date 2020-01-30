#include "TwindoSettings.h"

#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

bool TwindoSettings::load()
{
	boost::filesystem::path config_path(boost::filesystem::current_path());
	config_path.append(config_name);
	boost::property_tree::ptree pt;
	try {
		boost::property_tree::ini_parser::read_ini(config_path.string(), pt);
		place = pt.get<std::string>("params.code");
		api_path = pt.get<std::string>("params.api", api_default);
		log_path = pt.get<std::string>("log.path", log_path_default);
		log_level = pt.get<std::string>("log.level", log_level_default);
	}
	catch (const boost::property_tree::ini_parser::ini_parser_error & ) {
		return false;
	}
	return true;
}

TwindoSettings::TwindoSettings():api_path(),place()
{
}
