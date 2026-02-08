#pragma once

#include <string>
#include <vector>
#include <map>
#include <sstream>

enum class ParseResult {
    Success,    // The request is complete and valid.
    Incomplete, // The request is valid so far, needs more data.
    Error       // The request is malformed.
};


enum class HttpMethod {
    GET, POST, HEAD, PUT, DELETE_0, TRACE, OPTIONS, UNKNOWN
};

inline HttpMethod stringToHttpMethod(const std::string& methodStr);
inline std::string httpMethodToString(HttpMethod method);


class HttpRequest
{
public:
    ParseResult parse(const std::string& rawData);

    // --- Accessor Methods ---
    HttpMethod getMethod() const;
    const std::string& getRawUrl() const;
    const std::string& getPath() const;
    const std::vector<std::string>& getPathSegments() const;
    const std::map<std::string, std::string>& getQueryParams() const;
    const std::map<std::string, std::string>& getHeaders() const;
    const std::string& getBody() const;
    void setMethod(HttpMethod method);

    void clear();

private:
    bool parseRequestLine(const std::string& line);
    bool parseHeaders(const std::string& headersPart);
    bool parseUrl();

    HttpMethod m_method = HttpMethod::UNKNOWN;
    std::string m_rawUrl;
    std::string m_path;
    std::vector<std::string> m_pathSegments;
    std::map<std::string, std::string> m_queryParams;
    std::map<std::string, std::string> m_headers;
    std::string m_body;
};


// --- Inline Implementations for Helper Functions ---
inline HttpMethod stringToHttpMethod(const std::string& methodStr) {
    static const std::map<std::string, HttpMethod> methodMap = {
        {"GET",     HttpMethod::GET},
        {"POST",    HttpMethod::POST},
        {"HEAD",    HttpMethod::HEAD},
        {"PUT",     HttpMethod::PUT},
        {"DELETE",  HttpMethod::DELETE_0},
        {"OPTIONS", HttpMethod::OPTIONS},
        {"TRACE",   HttpMethod::TRACE}
    };
    auto it = methodMap.find(methodStr);
    return (it != methodMap.end()) ? it->second : HttpMethod::UNKNOWN;
}

inline std::string httpMethodToString(HttpMethod method) {
    switch (method) {
        case HttpMethod::GET:     return "GET";
        case HttpMethod::POST:    return "POST";
        case HttpMethod::HEAD:    return "HEAD";
        case HttpMethod::PUT:     return "PUT";
        case HttpMethod::DELETE_0:  return "DELETE";
        case HttpMethod::OPTIONS: return "OPTIONS";
        case HttpMethod::TRACE:   return "TRACE";
        default:                  return "UNKNOWN";
    }
}

// --- Inline Implementations for Accessors ---
inline HttpMethod HttpRequest::getMethod() const { return m_method; }
inline const std::string& HttpRequest::getRawUrl() const { return m_rawUrl; }
inline const std::string& HttpRequest::getPath() const { return m_path; }
inline const std::vector<std::string>& HttpRequest::getPathSegments() const { return m_pathSegments; }
inline const std::map<std::string, std::string>& HttpRequest::getQueryParams() const { return m_queryParams; }
inline const std::map<std::string, std::string>& HttpRequest::getHeaders() const { return m_headers; }
inline const std::string& HttpRequest::getBody() const { return m_body; }
inline void HttpRequest::setMethod(HttpMethod method) { m_method = method; }

