#pragma once

#include <string>
#include <map>
#include <chrono>
#include <iomanip>
#include <sstream>
#include "HttpStatusCodes.h"


class HttpResponse
{
public:
    // 1. Default constructor (for building piece by piece)
    HttpResponse() = default;

    // 2. Constructor for simple responses (e.g., 404 Not Found)
    explicit HttpResponse(HttpStatusCode code)
        : m_statusCode(code) {}

    // 3. Constructor for responses with a status code and a body
    HttpResponse(HttpStatusCode code, const std::string& body)
        : m_statusCode(code), m_body(body) {}

    // 4. Constructor for full control over the response
    HttpResponse(HttpStatusCode code, const std::string& body, const std::map<std::string, std::string>& headers)
        : m_statusCode(code), m_body(body), m_headers(headers) {}


    // --- Public Methods to modify the response ---
    void setStatusCode(HttpStatusCode code) {
        m_statusCode = code;
    }

    void addHeader(const std::string& key, const std::string& value) {
        m_headers[key] = value;
    }

    void setBody(const std::string& body) {
        m_body = body;
    }

    // --- Accessor methods ---
    HttpStatusCode getStatusCode() const {
        return m_statusCode;
    }

    std::string getBody() const {
        return m_body;
    }

    // --- The main method to serialize the object into a string ---
    std::string toString() const {
        std::string response;

        // Status Line
        response = "HTTP/1.1 " + std::to_string(static_cast<int>(m_statusCode)) + " " + getReasonPhrase(m_statusCode) + "\r\n";

        // Create a copy of headers to add/overwrite mandatory and default ones
        std::map<std::string, std::string> responseHeaders = m_headers;

        // --- Add default headers if they are not already set ---
        if (responseHeaders.find("Date") == responseHeaders.end()) {
            auto now = std::chrono::system_clock::now();
            auto time_t_now = std::chrono::system_clock::to_time_t(now);
            std::tm tm_buf;
            gmtime_s(&tm_buf, &time_t_now);
            std::stringstream ss;
            ss << std::put_time(&tm_buf, "%a, %d %b %Y %H:%M:%S GMT");
            responseHeaders["Date"] = ss.str();
        }
        if (responseHeaders.find("Server") == responseHeaders.end()) {
            responseHeaders["Server"] = "MySimpleWebServer";
        }
        if (responseHeaders.find("Connection") == responseHeaders.end()) {
            responseHeaders["Connection"] = "keep-alive";
        }
        if (!m_body.empty() && responseHeaders.find("Content-Type") == responseHeaders.end()) {
            responseHeaders["Content-Type"] = "application/octet-stream";
        }

        responseHeaders["Content-Length"] = std::to_string(m_body.length());

        for (const auto& header : responseHeaders) {
            response += header.first + ": " + header.second + "\r\n";
        }

        response += "\r\n";
        response += m_body;
        
        return response;
    }

private:
    HttpStatusCode m_statusCode = HttpStatusCode::Ok;
    std::map<std::string, std::string> m_headers;
    std::string m_body;
};

