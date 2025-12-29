#pragma once
#include <cstdlib>
#include <httplib.h>
#include <string>

namespace alpaca {

class Environment {
private:
  static std::string GetEnvVar(const char *key) {
    if (const char *v = std::getenv(key))
      return std::string(v);
    return {};
  }

public:
  Environment()
      : api_id_(GetEnvVar(env_api_key_id_)),
        api_secret_(GetEnvVar(env_api_secret_key_)),
        api_base_url_(GetEnvVar(env_api_base_url_)),
        api_data_url_(GetEnvVar(env_api_data_url_)) {}

  httplib::Headers GetAuthHeaders() const {
    return {
        {header_api_key_id_, api_id_},
        {header_api_secret_key_, api_secret_},
        {"Accept", "application/json"},
    };
  }

  const std::string &GetID() const { return api_id_; }
  const std::string &GetSecret() const { return api_secret_; }
  const std::string &GetBaseUrl() const { return api_base_url_; }
  const std::string &GetDataUrl() const { return api_data_url_; }

  static constexpr const char *GetIDEnvKey() { return env_api_key_id_; }
  static constexpr const char *GetSecretEnvKey() { return env_api_secret_key_; }
  static constexpr const char *GetBaseUrlEnvKey() { return env_api_base_url_; }
  static constexpr const char *GetDataUrlEnvKey() { return env_api_data_url_; }

private:
  std::string api_id_;
  std::string api_secret_;
  std::string api_base_url_;
  std::string api_data_url_;

  static inline constexpr const char *env_api_key_id_ = "APCA_API_KEY_ID";
  static inline constexpr const char *env_api_secret_key_ =
      "APCA_API_SECRET_KEY";
  static inline constexpr const char *env_api_base_url_ = "APCA_API_BASE_URL";
  static inline constexpr const char *env_api_data_url_ = "APCA_API_DATA_URL";

  static inline constexpr const char *header_api_key_id_ = "APCA-API-KEY-ID";
  static inline constexpr const char *header_api_secret_key_ =
      "APCA-API-SECRET-KEY";
};

} // namespace alpaca
