#pragma once

#include "HttpRequest.h"
#include "HttpResponse.h"
#include <string>

class IEndpoint {
public:
    // Handles an incoming request and returns a response.
    virtual HttpResponse handle(const HttpRequest& request) = 0;

    // Provides a short, human-readable description of what the endpoint does.
    virtual std::string getDescription() const = 0;

    virtual ~IEndpoint() = default;
};

