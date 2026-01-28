#pragma once
#include <string>
#include <vector>

std::string http_get(const std::string& url);
std::string http_post(const std::string& url, const std::string& body,
                      const std::vector<std::string>& headers);