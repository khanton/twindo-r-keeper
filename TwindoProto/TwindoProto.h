#pragma once
#include <stdint.h>

#include "TwindoSettings.h"

class TwindoProto
{
private:
	TwindoSettings config;
	std::string user;

public:
	TwindoProto();

	bool init();

	bool setUser(const uint64_t card, std::string& _user);

	bool get(const std::string& user, std::string& name);
	bool balance(const std::string& user, const double summa, unsigned long& balance, unsigned long& allowed, unsigned long& allowed_in_currency);
	bool commit(const std::string& user, const double summa);

	void error(const std::string&);
	void trace(const std::string&);

};

