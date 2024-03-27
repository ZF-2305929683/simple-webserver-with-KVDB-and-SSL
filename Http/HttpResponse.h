#pragma once

#include <map>
#include <string>

class Buffer;
class HttpResponse 
{
 public:
  enum class HttpStatusCode
  {
    Unknown,
    success = 200,
    MovedPermanently = 301,
    BadRequest = 400,
    NotFound = 404,
  };

  explicit HttpResponse(bool close): statusCode_(HttpStatusCode::Unknown),closeConnection_(close){}

  void setStatusCode(HttpStatusCode code){ statusCode_ = code; }

  void setStatusMessage(const std::string& message){ statusMessage_ = message; }

  void setCloseConnection(bool on){ closeConnection_ = on; }

  bool closeConnection() const{ return closeConnection_; }

  void setContentType(const std::string& contentType){ addHeader("Content-Type", contentType); }

  // FIXME: replace string with StringPiece
  void addHeader(const std::string& key, const std::string& value){ headers_[key] = value; }

  void setBody(const std::string& body){ body_ = body; }

  void appendToBuffer(Buffer* output) const;

 private:
  std::map<std::string, std::string> headers_;
  HttpResponse::HttpStatusCode statusCode_;
  // FIXME: add http version
  std::string statusMessage_;
  bool closeConnection_;
  std::string body_;
};