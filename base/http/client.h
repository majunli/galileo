#ifndef __GALILEO_BASE_HTTP_CLIENT_H__
#define __GALILEO_BASE_HTTP_CLIENT_H__

#include "../basetypes.h"
#include "../extendable_packet.h"
#include <vector>

class CURL;

namespace http {

// 一个简单的 HTTP 客户端，可发送 GET/POST 请求。
class client {
public:
  typedef extendable_packet<true, uint32_t, 64U, false> body_t;

  client();
  ~client();

  // 设置默认的 User-Agent
  void set_user_agent(const char *user_agent);

  // 设置代理
  // 格式：127.0.0.1:8888
  // 不使用代理：set_proxy("")
  void set_proxy(const char *proxy);

  // 以秒为单位，设置连接超时时间。
  void set_connect_timeout(int seconds);

  // 以秒为单位，设置请求响应超时时间。
  void set_timeout(int seconds);

  // 发送 GET 请求
  bool get(const char *url, const std::vector<std::string> &custom_headers = std::vector<std::string>());

  // 发送 POST 请求
  bool post(const char *url, const std::vector<std::string> &custom_headers, const char *content_type, const char *content);

  // 返回 HTTP 状态码
  const int status_code() const;

  // 返回当前 URL
  const char *current_url() const;

  // 返回响应数据
  body_t *body() const { return body_; }

private:
  std::vector<std::string> headers_;
  CURL *curl_;
  body_t *body_;

  DISALLOW_COPY_AND_ASSIGN(client);
};

}

#endif // !__GALILEO_BASE_HTTP_CLIENT_H__
