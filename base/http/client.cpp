#include "client.h"

#include <curl/curl.h>

namespace {

static bool curl_global_initialized = false;

// 配合静态变量 global_http_client 保证 curl_global_init 方法的调用
class global_http_client {
public:
  global_http_client() {
    if (!curl_global_initialized) {
      curl_global_initialized = curl_global_init(CURL_GLOBAL_ALL) == CURLE_OK;
    }
  }

  ~global_http_client() {
    if (curl_global_initialized) {
      curl_global_cleanup();
    }
  }
};

static global_http_client global_curl;

// 响应写入回调
static size_t write_callback(char *buffer, size_t size, size_t nmemb, http::client::body_t *body) {
  if (NULL == body || NULL == buffer) {
    return 0;
  }

  body->append_bytes(buffer, size * nmemb);
  return size * nmemb;
}

} // namespace


namespace http {

client::client(const char *user_agent) {
  curl_ = curl_easy_init();
  if (curl_ != NULL) {
    curl_easy_setopt(curl_, CURLOPT_ACCEPT_ENCODING, "gzip, deflate");
    curl_easy_setopt(curl_, CURLOPT_USERAGENT, user_agent);
    curl_easy_setopt(curl_, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl_, CURLOPT_CONNECTTIMEOUT, 30L);
    curl_easy_setopt(curl_, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl_, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl_, CURLOPT_MAXREDIRS, 20L);

    curl_easy_setopt(curl_, CURLOPT_READFUNCTION, NULL);
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &body_);

    curl_easy_setopt(curl_, CURLOPT_COOKIEFILE, "");				//设置cookie文件保存路径,必须要执行,否则不能获得cookie数据.
  }
}

client::~client() {
  if (curl_ != NULL) {
    curl_easy_cleanup(curl_);
    curl_ = NULL;
  }
}

void client::set_user_agent(const char *user_agent) {
  curl_easy_setopt(curl_, CURLOPT_USERAGENT, user_agent);
}

void client::set_proxy(const char *proxy) {
  curl_easy_setopt(curl_, CURLOPT_PROXY, proxy);
}

void client::set_connect_timeout(int seconds) {
  curl_easy_setopt(curl_, CURLOPT_CONNECTTIMEOUT, seconds);
}

void client::set_timeout(int seconds) {
  curl_easy_setopt(curl_, CURLOPT_TIMEOUT, seconds);
}

bool client::get(const char *url, const std::vector<std::string> &custom_headers /*= std::vector<std::string>()*/) {
  body_.clear();

  curl_easy_setopt(curl_, CURLOPT_URL, url);
  curl_easy_setopt(curl_, CURLOPT_POST, 0L);

  struct curl_slist *headers = NULL;

  for (std::vector<std::string>::const_iterator iter = headers_.begin(); iter != headers_.end(); ++iter) {
    headers = curl_slist_append(headers, iter->c_str());
  }

  for (std::vector<std::string>::const_iterator iter = custom_headers.begin(); iter != custom_headers.end(); ++iter) {
    headers = curl_slist_append(headers, iter->c_str());
  }

  if (headers != NULL) {
    curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers);
  }

  CURLcode code = curl_easy_perform(curl_);

  if (headers != NULL) {
    curl_slist_free_all(headers);
  }

  return code == CURLE_OK;
}

bool client::post(const char *url, const std::vector<std::string> &custom_headers, const char *content_type, const char *content) {
  body_.clear();

  curl_easy_setopt(curl_, CURLOPT_URL, url);
  curl_easy_setopt(curl_, CURLOPT_POST, 1L);

  struct curl_slist *headers = NULL;

  for (std::vector<std::string>::const_iterator iter = headers_.begin(); iter != headers_.end(); ++iter) {
    headers = curl_slist_append(headers, iter->c_str());
  }

  for (std::vector<std::string>::const_iterator iter = custom_headers.begin(); iter != custom_headers.end(); ++iter) {
    headers = curl_slist_append(headers, iter->c_str());
  }

  std::string content_type_field = "Content-Type: ";
  content_type_field.append(content_type);
  headers = curl_slist_append(headers, content_type_field.c_str());

  if (headers != NULL) {
    curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers);
  }

  if (content != NULL) {
    curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, content);
  }

  CURLcode code = curl_easy_perform(curl_);

  return code == CURLE_OK;
}

const int client::status_code() const {
  int response_code;
  curl_easy_getinfo(curl_, CURLINFO::CURLINFO_RESPONSE_CODE, &response_code);
  return response_code;
}

const char * client::current_url() const {
  char *effective_url;
  curl_easy_getinfo(curl_, CURLINFO::CURLINFO_EFFECTIVE_URL, &effective_url);
  return effective_url;
}

} // namespace http

