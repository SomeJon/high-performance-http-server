#include "Endpoints.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <sstream>

OptionsEndpoint::OptionsEndpoint(const std::map<HttpMethod, std::string>& supportedMethods)
    : m_supportedMethods(supportedMethods) {}

HttpResponse OptionsEndpoint::handle(const HttpRequest& request) {
    HttpResponse response(HttpStatusCode::Ok);
    std::string allowHeaderValue;
    std::string body = "<html><head><title>Allowed Options</title></head><body><h1>Allowed methods for " + request.getPath() + "</h1><ul>";

    // Add this endpoint's own info to the map for the response body.
    m_supportedMethods[HttpMethod::OPTIONS] = getDescription();

    for (const auto& pair : m_supportedMethods) {
        if (!allowHeaderValue.empty()) {
            allowHeaderValue += ", ";
        }
        allowHeaderValue += httpMethodToString(pair.first);
        body += "<li><b>" + httpMethodToString(pair.first) + ":</b> " + pair.second + "</li>";
    }
    body += "</ul></body></html>";

    response.addHeader("Allow", allowHeaderValue);
    response.addHeader("Content-Type", "text/html");
    response.setBody(body);
    return response;
}

std::string OptionsEndpoint::getDescription() const {
    return "Provides a list of allowed methods and their descriptions for this endpoint.";
}


// --- HomeEndpoint Implementation ---
HttpResponse HomeEndpoint::handle(const HttpRequest& request) {
    // Default to English
    std::string lang = "en";
    if (request.getQueryParams().count("lang")) {
        lang = request.getQueryParams().at("lang");
    }

    // --- Language-Specific Strings ---
    std::string greeting, pageStatus, imageText;
    std::string direction = "ltr"; // Default direction

    if (lang == "he") {
        greeting = "שלום עולם!";
        pageStatus = "הדף מוצג כעת בעברית.";
        direction = "rtl";
    } else if (lang == "fr") {
        greeting = "Bonjour le monde!";
        pageStatus = "Cette page est actuellement en français.";
    } else { // Default to English
        greeting = "Hello World!";
        pageStatus = "This page is currently in English.";
    }

    // --- HTML Body Construction ---
    std::string body = R"(
<!DOCTYPE html>
<html lang=")" + lang + R"(" dir=")" + direction + R"(">
<head>
    <meta charset="UTF-8">
    <title>Web Server</title>
    <style>
        body { font-family: sans-serif; background-color: #f0f2f5; text-align: center; margin: 40px; }
        .container { background-color: #fff; padding: 30px; border-radius: 10px; box-shadow: 0 4px 8px rgba(0,0,0,0.1); display: inline-block; }
        p { color: #555; }
        img { max-width: 100%; border-radius: 8px; margin-top: 20px; }
    </style>
</head>
<body>
    <div class='container'>
        <h1>)" + greeting + R"(</h1>
        <p>)" + pageStatus + R"(</p>
    </div>
</body>
</html>)";

    // --- Build and Return Response ---
    HttpResponse response(HttpStatusCode::Ok, body);
    response.addHeader("Content-Type", "text/html; charset=UTF-8");
    return response;
}
std::string HomeEndpoint::getDescription() const {
    return "Returns a greeting webpage. Supports 'lang' query parameter (en, he, fr).";
}

// --- PostMessageEndpoint Implementation ---
HttpResponse PostMessageEndpoint::handle(const HttpRequest& request) {
    // Using std::endl forces the output buffer to flush immediately.
    std::cout << "--- POST Request Body ---" << std::endl
              << request.getBody() << std::endl
              << "---------------------------" << std::endl;
    return HttpResponse(HttpStatusCode::NoContent);
}
std::string PostMessageEndpoint::getDescription() const {
    return "Accepts a text body and prints it to the server console.";
}


// --- File Endpoint Implementations ---
HttpResponse PutFileEndpoint::handle(const HttpRequest& request) {
    if (request.getPathSegments().size() < 2) return HttpResponse(HttpStatusCode::BadRequest, "Missing filename.");
    std::filesystem::create_directory("files");
    std::ofstream outFile("files/" + request.getPathSegments()[1], std::ios::binary);
    if (!outFile) return HttpResponse(HttpStatusCode::InternalServerError, "Could not open file.");
    outFile << request.getBody();
    return HttpResponse(HttpStatusCode::Created, "File created.");
}
std::string PutFileEndpoint::getDescription() const { return "Creates or replaces a file: /file/{filename}."; }

HttpResponse GetFileEndpoint::handle(const HttpRequest& request) {
    if (request.getPathSegments().size() < 2) return HttpResponse(HttpStatusCode::BadRequest, "Missing filename.");
    std::string filename = "files/" + request.getPathSegments()[1];
    if (!std::filesystem::exists(filename)) return HttpResponse(HttpStatusCode::NotFound, "File not found.");
    std::ifstream inFile(filename, std::ios::binary);
    std::stringstream buffer;
    buffer << inFile.rdbuf();
    HttpResponse response(HttpStatusCode::Ok, buffer.str());
    response.addHeader("Content-Type", "application/octet-stream");
    return response;
}
std::string GetFileEndpoint::getDescription() const { return "Retrieves a file: /file/{filename}."; }

HttpResponse DeleteFileEndpoint::handle(const HttpRequest& request) {
    if (request.getPathSegments().size() < 2) return HttpResponse(HttpStatusCode::BadRequest, "Missing filename.");
    std::string filename = "files/" + request.getPathSegments()[1];
    if (!std::filesystem::exists(filename)) return HttpResponse(HttpStatusCode::NotFound, "File not found.");
    if (std::remove(filename.c_str()) != 0) return HttpResponse(HttpStatusCode::InternalServerError, "Error deleting file.");
    return HttpResponse(HttpStatusCode::Ok, "File deleted.");
}
std::string DeleteFileEndpoint::getDescription() const { return "Deletes a file: /file/{filename}."; }


// --- TraceEndpoint Implementation ---
HttpResponse TraceEndpoint::handle(const HttpRequest& request) {
    std::string echoedRequest;
    echoedRequest += httpMethodToString(request.getMethod()) + " " + request.getRawUrl() + " HTTP/1.1\r\n";
    for (const auto& header : request.getHeaders()) {
        echoedRequest += header.first + ": " + header.second + "\r\n";
    }
    echoedRequest += "\r\n";
    HttpResponse response(HttpStatusCode::Ok, echoedRequest);
    response.addHeader("Content-Type", "message/http");
    return response;
}
std::string TraceEndpoint::getDescription() const { return "Echoes the received request headers back to the client."; }

