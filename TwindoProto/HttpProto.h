#pragma once
#include <string>
#include <list>
#include <utility>
#include <windows.h>

class HttpProto
{
private:
	typedef std::list< std::pair<std::wstring, std::wstring> > headers_list_t;
	std::wstring m_base;
	headers_list_t m_headers;
public:
	HttpProto(const std::wstring &base);

	void addHeader(const std::wstring &name, const std::wstring &value);

	int get(const std::wstring& url, std::string & response);
	int post(const std::wstring& url, const std::string & body, std::string & response);

	static std::string wstring_to_utf8(const std::wstring& str);
	static std::wstring utf8_to_wstring(const std::string& str);

private:
	bool request(const std::wstring &url, const std::wstring &verb, const std::wstring &headers, const std::string & body, DWORD &statusCode, std::string & response);
	void buildHeaders(std::wstring& headers);
};

#define S2W(str) (HttpProto::utf8_to_wstring(str))
#define W2S(str) (HttpProto::wstring_to_utf8(str))