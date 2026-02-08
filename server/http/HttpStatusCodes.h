#pragma once
#include <string>

// Enum class for type-safe HTTP status codes
enum class HttpStatusCode {
    // 2xx Success
    Ok = 200,
    Created = 201,
    NoContent = 204,

    // 4xx Client Error
    BadRequest = 400,
    NotFound = 404,

    // 5xx Server Error
    InternalServerError = 500,
    NotImplemented = 501
};

// Helper function to get the standard reason phrase for a status code.
inline std::string getReasonPhrase(HttpStatusCode code) {
    switch (code) {
        case HttpStatusCode::Ok:                    return "OK";
        case HttpStatusCode::NoContent:             return "No Content";
        case HttpStatusCode::Created:               return "Created";
        case HttpStatusCode::BadRequest:            return "Bad Request";
        case HttpStatusCode::NotFound:              return "Not Found";
        case HttpStatusCode::InternalServerError:   return "Internal Server Error";
        case HttpStatusCode::NotImplemented:        return "Not Implemented";
        default:                                    return "Unknown Status";
    }
}
