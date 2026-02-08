#pragma once

#include "IEndpoint.h"
#include <vector>
#include <map>


class OptionsEndpoint final : public IEndpoint {
public:
    OptionsEndpoint(const std::map<HttpMethod, std::string>& supportedMethods);

    HttpResponse handle(const HttpRequest& request) override;
    std::string getDescription() const override;

private:
    std::map<HttpMethod, std::string> m_supportedMethods;
};

class HomeEndpoint final : public IEndpoint {
public:
    HttpResponse handle(const HttpRequest& request) override;
    std::string getDescription() const override;
};

class PostMessageEndpoint final : public IEndpoint {
public:
    HttpResponse handle(const HttpRequest& request) override;
    std::string getDescription() const override;
};

class PutFileEndpoint final : public IEndpoint {
public:
    HttpResponse handle(const HttpRequest& request) override;
    std::string getDescription() const override;
};

class GetFileEndpoint final : public IEndpoint {
public:
    HttpResponse handle(const HttpRequest& request) override;
    std::string getDescription() const override;
};

class DeleteFileEndpoint final : public IEndpoint {
public:
    HttpResponse handle(const HttpRequest& request) override;
    std::string getDescription() const override;
};

class TraceEndpoint final : public IEndpoint {
public:
    HttpResponse handle(const HttpRequest& request) override;
    std::string getDescription() const override;
};

