#pragma once
#include <expected>
#include <glaze/glaze.hpp>
#include <httplib.h>

namespace alpaca {

struct Status {
  int status;
  std::string body;
};

class HttpClient {
public:
  explicit HttpClient(const std::string &host) : cli(host) {}

  std::expected<alpaca::Status, std::string>
  Get(const std::string &path, const httplib::Headers &headers) {
    auto resp = cli.Get(path.c_str(), headers);
    if (!resp) {
      return std::unexpected(
          "Error: path or headers are wrong and/or ill-formed");
    }

    return Status{resp->status, resp->body};
  }

private:
  httplib::SSLClient cli;
};

}; // namespace alpaca
