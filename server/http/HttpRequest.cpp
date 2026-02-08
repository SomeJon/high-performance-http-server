#include "HttpRequest.h"

// The main parsing method.
ParseResult HttpRequest::parse(const std::string& rawData) {
    clear(); // Reset state for parsing a new request

    const std::string EOH = "\r\n\r\n";
    size_t headersEnd = rawData.find(EOH);
    if (headersEnd == std::string::npos) {
        return ParseResult::Incomplete;
    }

    std::string headersPart = rawData.substr(0, headersEnd);
    m_body = rawData.substr(headersEnd + EOH.length());

    size_t requestLineEnd = headersPart.find("\r\n");
    if (requestLineEnd == std::string::npos) {
        return ParseResult::Error;
    }

    std::string requestLine = headersPart.substr(0, requestLineEnd);
    if (!parseRequestLine(requestLine)) {
        return ParseResult::Error;
    }
    
    headersPart.erase(0, requestLineEnd + 2);
    if (!parseHeaders(headersPart)) {
        return ParseResult::Error;
    }
    
    if (!parseUrl()) {
        return ParseResult::Error;
    }

    // Check if the body is complete
    if (m_headers.count("Content-Length")) {
        try {
            size_t expectedLength = std::stoul(m_headers.at("Content-Length"));
            if (m_body.length() < expectedLength) {
                return ParseResult::Incomplete; // Body is not fully received yet
            }
            if (m_body.length() > expectedLength) {
                m_body.resize(expectedLength); // Trim any extra data
            }
        } catch (const std::exception&) {
            return ParseResult::Error; // Malformed Content-Length
        }
    }

    return ParseResult::Success;
}

// Resets the state of the request object to be reused.
void HttpRequest::clear() {
    m_method = HttpMethod::UNKNOWN;
    m_rawUrl.clear();
    m_path.clear();
    m_pathSegments.clear();
    m_queryParams.clear();
    m_headers.clear();
    m_body.clear();
}

// --- Private parsing helper functions ---
bool HttpRequest::parseRequestLine(const std::string& line) {
    std::stringstream requestLineStream(line);
    std::string methodStr;
    requestLineStream >> methodStr >> m_rawUrl;
    m_method = stringToHttpMethod(methodStr);
    return m_method != HttpMethod::UNKNOWN;
}

bool HttpRequest::parseHeaders(const std::string& headersPart) {
    std::stringstream headerStream(headersPart);
    std::string headerLine;
    while (std::getline(headerStream, headerLine) && !headerLine.empty() && headerLine != "\r") {
        size_t colonPos = headerLine.find(": ");
        if (colonPos != std::string::npos) {
            std::string key = headerLine.substr(0, colonPos);
            std::string value = headerLine.substr(colonPos + 2);
            // Trim potential trailing \r from value
            if (!value.empty() && value.back() == '\r') {
                value.pop_back();
            }
            m_headers[key] = value;
        }
    }
    return true;
}

bool HttpRequest::parseUrl() {
    size_t queryPos = m_rawUrl.find('?');
    if (queryPos != std::string::npos) {
        m_path = m_rawUrl.substr(0, queryPos);
        std::string queryString = m_rawUrl.substr(queryPos + 1);
        std::stringstream queryStream(queryString);
        std::string param;
        while (std::getline(queryStream, param, '&')) {
            size_t equalPos = param.find('=');
            if (equalPos != std::string::npos) {
                m_queryParams[param.substr(0, equalPos)] = param.substr(equalPos + 1);
            }
        }
    } else {
        m_path = m_rawUrl;
    }

    std::stringstream pathStream(m_path);
    std::string segment;
    while (std::getline(pathStream, segment, '/')) {
        if (!segment.empty()) {
            m_pathSegments.push_back(segment);
        }
    }
    return true;
}
