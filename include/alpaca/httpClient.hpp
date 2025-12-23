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
private:
  static std::string to_string(httplib::Error e) {
    switch (e) {
    case httplib::Error::Success:
      return "Success";
    case httplib::Error::Unknown:
      return "Unknown";
    case httplib::Error::Connection:
      return "Connection";
    case httplib::Error::BindIPAddress:
      return "BindIPAddress";
    case httplib::Error::Read:
      return "Read";
    case httplib::Error::Write:
      return "Write";
    case httplib::Error::ExceedRedirectCount:
      return "ExceedRedirectCount";
    case httplib::Error::Canceled:
      return "Canceled";
    case httplib::Error::SSLConnection:
      return "SSLConnection";
    case httplib::Error::SSLLoadingCerts:
      return "SSLLoadingCerts";
    case httplib::Error::SSLServerVerification:
      return "SSLServerVerification";
    default:
      return std::format("Error({})", static_cast<int>(e));
    }
  }

public:
  explicit HttpClient(const std::string &host) : cli_(host) {
    cli_.set_connection_timeout(5);
    cli_.set_read_timeout(10);
    cli_.set_write_timeout(10);
    cli_.set_keep_alive(true);
  }

  std::expected<alpaca::Status, std::string>
  Get(const std::string &path, const httplib::Headers &headers) {
    if (!cli_.is_valid()) {
      return std::unexpected(
          "SSLClient is not valid (bad host/port or SSL init failed).");
    }

    auto resp = cli_.Get(path, headers);
    if (!resp) {
      return std::unexpected(
          std::format("Transport error: {}", to_string(resp.error())));
    }

    return Status{resp->status, resp->body};
  }

  std::expected<alpaca::Status, std::string>
  Post(const std::string &path, const httplib::Headers &headers,
       const std::string &body, const std::string &content_type) {
    auto resp = cli_.Post(path, headers, body, content_type);
    if (!resp) {
      return std::unexpected(
          "Error: path or headers are wrong and/or ill-formed");
    }

    return Status{resp->status, resp->body};
  }

  std::expected<alpaca::Status, std::string>
  Delete(const std::string &path, const httplib::Headers &headers) {
    auto resp = cli_.Delete(path, headers);
    if (!resp) {
      return std::unexpected(
          "Error: path or headers are wrong and/or ill-formed");
    }

    return Status{resp->status, resp->body};
  }

private:
  httplib::SSLClient cli_;
};

}; // namespace alpaca
