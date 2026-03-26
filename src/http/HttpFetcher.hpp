#pragma once

#include <curl/curl.h>
#include <string>

class HttpFetcher {
public:
  HttpFetcher() {
    curl_ = curl_easy_init();
    // Set common options once
    curl_easy_setopt(curl_, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl_, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl_, CURLOPT_USERAGENT, "StationC3/1.0");
    // Enable keep‑alive
    curl_easy_setopt(curl_, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(curl_, CURLOPT_TCP_KEEPIDLE, 120L);
    curl_easy_setopt(curl_, CURLOPT_TCP_KEEPINTVL, 60L);
    // Set a custom write function
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response_);

    // use http2
    curl_easy_setopt(curl_, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);
  }

  ~HttpFetcher() {
    if (curl_)
      curl_easy_cleanup(curl_);
  }

  std::string get(const std::string &url) {
    response_.clear();

    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
    CURLcode res = curl_easy_perform(curl_);
    if (res != CURLE_OK) {
      throw std::runtime_error("curl error: " +
                               std::string(curl_easy_strerror(res)));
    }
    return response_;
  }

  std::string url_encode(const std::string &value) {
    char *encoded = curl_easy_escape(curl_, value.c_str(), value.length());
    std::string result(encoded);
    curl_free(encoded);
    return result;
  }

private:
  static size_t writeCallback(void *contents, size_t size, size_t nmemb,
                              std::string *out) {
    size_t total = size * nmemb;
    out->append(static_cast<char *>(contents), total);
    return total;
  }

  CURL *curl_;
  std::string response_;
};