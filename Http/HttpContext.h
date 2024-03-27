#pragma once

#include "HttpRequest.h"


class Buffer;

class HttpContext
{
 public:
  enum class HttpRequestParseState
  {
    ExpectRequestLine,
    ExpectHeaders,
    ExpectBody,
    GotAll,
  };

  HttpContext(): state_(HttpRequestParseState::ExpectRequestLine){}

  bool gotAll() const { return state_ == HttpRequestParseState::GotAll; }

  void reset()
  {
    state_ = HttpRequestParseState::ExpectRequestLine;
    HttpRequest dummy;
    request_.swap(dummy);
  }

  const HttpRequest& request() const{ return request_; }

  HttpRequest& request(){ return request_; }

  bool parseRequest(Buffer* buf, Timestamp receiveTime);

private:
  bool processRequestLine(const char* begin, const char* end);


private:
  HttpRequestParseState state_;
  HttpRequest request_;
};
