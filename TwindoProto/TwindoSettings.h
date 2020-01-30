#pragma once
#include <string>

class TwindoSettings
{
public:
	std::string api_path;
	std::string place;
	std::string log_path;
	std::string log_level;

	bool load();
	TwindoSettings();

private:
	const char *config_name = "twindo.ini";
	const char* api_default = "https://twindo.ru/api/v3";
	const char* log_level_default = "info";
	const char* log_path_default = "c:\\ucs\\manager\\farcards\\twindo.log";
};

