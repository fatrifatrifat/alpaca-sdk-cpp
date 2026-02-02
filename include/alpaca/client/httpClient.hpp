#pragma once
#include <alpaca/client/environment.hpp>
#include <alpaca/utils/utils.hpp>
#include <expected>
#include <format>
#include <glaze/glaze.hpp>
#include <httplib.h>
#include <optional>
#include <print>
#include <string>

namespace alpaca {

enum class ErrorCode {
  Success,
  Unknown,
  Connection,
  BindIPAddress,
  Read,
  Write,
  ExceedRedirectCount,
  Canceled,
  SSLConnection,
  SSLLoadingCerts,
  SSLServerVerification,
  HTTPCode,
  InvalidClient,
  JSONParsing,
  Transport,
  IllArgument,
};

struct APIError {
  ErrorCode code;
  std::string message;
  std::optional<int> status = std::nullopt;
};

enum class Req {
  GET,
  POST,
  DELETE,
  PATCH,
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
  explicit HttpClient(const std::string &host, const httplib::Headers &headers)
      : cli_(host), headers_(headers) {}

  template <typename T>
  std::expected<T, APIError>
  Request(Req type, const std::string &path,
          std::optional<std::string> body = std::nullopt,
          std::optional<std::string> content_type = std::nullopt) {
    if (!cli_.is_valid()) {
      return std::unexpected(APIError{
          ErrorCode::InvalidClient,
          "SSLClient is not valid (bad host/port or SSL init failed)."});
    }

    httplib::Result resp;
    switch (type) {
    case Req::GET:
      resp = cli_.Get(path, headers_);
      break;
    case Req::POST:
      if (!body || !content_type) {
        return std::unexpected(
            alpaca::APIError{alpaca::ErrorCode::IllArgument,
                             "POST requires body and content_type"});
      }
      resp = cli_.Post(path, headers_, *body, *content_type);
      break;
    case Req::DELETE:
      resp = cli_.Delete(path, headers_);
      break;
    case Req::PATCH:
      if (!body || !content_type) {
        return std::unexpected(
            alpaca::APIError{alpaca::ErrorCode::IllArgument,
                             "PATCH requires body and content_type"});
      }
      resp = cli_.Patch(path, headers_, *body, *content_type);
      break;
    }

    if (!resp) {
      return std::unexpected(
          APIError{ErrorCode::Transport, to_string(resp.error())});
    }

    if (!utils::IsSuccess(resp->status)) {
      return std::unexpected(
          APIError{ErrorCode::HTTPCode, resp->body, resp->status});
    }

    if (resp->body.empty()) {
      return T{};
    }

    T obj;
    auto error = glz::read_json(obj, resp->body);
    if (error) {
      return std::unexpected(APIError{ErrorCode::JSONParsing,
                                      glz::format_error(error, resp->body),
                                      resp->status});
    }

    return obj;
  }

private:
  const httplib::Headers headers_;
  httplib::SSLClient cli_;
};

}; // namespace alpaca

template <> struct std::formatter<alpaca::APIError> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  auto format(const alpaca::APIError &e, std::format_context &ctx) const {
    if (e.status) {
      return std::format_to(ctx.out(), "[{}, {}, {}]", static_cast<int>(e.code),
                            e.message, *e.status);
    }
    return std::format_to(ctx.out(), "[{}, {}]", static_cast<int>(e.code),
                          e.message);
  }
};
