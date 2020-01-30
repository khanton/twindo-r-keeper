#include "HttpProto.h"
#include <codecvt>

#include "winhttp.h"
#pragma comment(lib, "winhttp.lib")

HttpProto::HttpProto(const std::wstring& base): m_base(base)
{
}

void HttpProto::addHeader(const std::wstring &name,const std::wstring &value)
{
    m_headers.push_back(std::make_pair(name, value));
}

int HttpProto::get(const std::wstring& url, std::string & response)
{
    std::wstring headers;
    DWORD statusCode = 0;

    buildHeaders(headers);

    if (request(m_base + url, L"GET", headers, "", statusCode, response)) {
        return statusCode;
    }

    return 0;
}

int HttpProto::post(const std::wstring& url, const std::string & body, std::string & response)
{
    std::wstring headers;
    DWORD statusCode = 0;

    buildHeaders(headers);

    if (request(m_base + url, L"POST", headers, body, statusCode, response)) {
        return statusCode;
    }

    return 0;
}

std::string HttpProto::wstring_to_utf8(const std::wstring& str)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
    return myconv.to_bytes(str);
}

std::wstring HttpProto::utf8_to_wstring(const std::string& str)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
    return myconv.from_bytes(str);
}

bool HttpProto::request(const std::wstring& url, const std::wstring& verb, const std::wstring& headers, const std::string & body, DWORD& statusCode, std::string & response){
    bool result = false;
    URL_COMPONENTS urlComponents;

    memset(&urlComponents, 0, sizeof(urlComponents));
    urlComponents.dwStructSize = sizeof(urlComponents);
    urlComponents.dwUserNameLength = 1;
    urlComponents.dwPasswordLength = 1;
    urlComponents.dwHostNameLength = 1;
    urlComponents.dwUrlPathLength = 1;

    if (!WinHttpCrackUrl(url.c_str(), 0, 0, &urlComponents))
    {
        return result;
    }

    std::wstring host(urlComponents.lpszHostName, urlComponents.dwHostNameLength);
    std::wstring path(urlComponents.lpszUrlPath, urlComponents.dwUrlPathLength);

    HINTERNET hSession = WinHttpOpen(TEXT("TwindoCashDeskRequest/1.0.0"), 
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, NULL, NULL, 0);

    if (hSession) {

        DWORD protocols = WINHTTP_FLAG_SECURE_PROTOCOL_SSL3 | WINHTTP_FLAG_SECURE_PROTOCOL_TLS1 |
            WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_1 | WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_2;

        WinHttpSetOption(hSession, WINHTTP_OPTION_SECURE_PROTOCOLS, &protocols, sizeof(protocols));

        HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(), urlComponents.nPort, 0);

        if (hConnect) {
            HINTERNET hRequest = WinHttpOpenRequest(hConnect, verb.c_str(), path.c_str(), NULL, NULL, NULL,
                urlComponents.nScheme == INTERNET_SCHEME_HTTPS ? WINHTTP_FLAG_SECURE : 0);

            if (hRequest) {

                while (true) {

                    if (!WinHttpSendRequest(hRequest, headers.c_str(), headers.length(),
                        NULL, 0, body.length(), 0)) {
                        result = false;
                        break;
                    }

                    DWORD dwBytesRead;
                    
                    if (body.length() > 0) {
                        if (!WinHttpWriteData(hRequest, body.c_str(), body.length(), &dwBytesRead)) {
                            result = false;
                            break;
                        }
                    }

                    if (!WinHttpReceiveResponse(hRequest, NULL)) {
                        result = false;
                        break;
                    }

                    DWORD dwLenght = sizeof(statusCode);

                    if (!WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                        NULL, &statusCode, &dwLenght, NULL)) {
                        break;
                    }

                    DWORD dwReadBytes = 0;
                    char buffer[4096];

                    if (WinHttpReadData(hRequest, buffer, sizeof(buffer), &dwReadBytes) && (dwReadBytes)) {

                        response = std::string(buffer, dwReadBytes);
                        result = true;
                    }

                    break;
                }

                WinHttpCloseHandle(hRequest);
            }
        }

        WinHttpCloseHandle(hSession);
    }

    return result;
}

void HttpProto::buildHeaders(std::wstring& headers)
{
    headers.clear();
    for (headers_list_t::iterator it = m_headers.begin(); it != m_headers.end(); it++) {
       headers += it->first + L": " + it->second + L"\n";
    }

    if (headers.length() > 1) {
        headers.resize(headers.length() - 1);
    }
}
