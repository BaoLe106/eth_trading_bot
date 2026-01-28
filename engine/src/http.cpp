#include "include/http.hpp"
#include <curl/curl.h>
#include <stdexcept>

static size_t wr(char* p, size_t s, size_t n, void* u){ ((std::string*)u)->append(p,s*n); return s*n; }

std::string http_get(const std::string& url) {
    CURL* c = curl_easy_init(); if(!c) throw std::runtime_error("curl init");
    std::string out;
    curl_easy_setopt(c, CURLOPT_URL, url.c_str());
    curl_easy_setopt(c, CURLOPT_WRITEFUNCTION, wr);
    curl_easy_setopt(c, CURLOPT_WRITEDATA, &out);
    curl_easy_setopt(c, CURLOPT_TIMEOUT, 15L);
    auto rc = curl_easy_perform(c);
    curl_easy_cleanup(c);
    if(rc != CURLE_OK) throw std::runtime_error("GET failed");
    return out;
}

std::string http_post(const std::string& url, const std::string& body,
                      const std::vector<std::string>& headers) {
    CURL* c = curl_easy_init(); if(!c) throw std::runtime_error("curl init");
    std::string out; struct curl_slist* hs = nullptr;
    for (auto& h : headers) hs = curl_slist_append(hs, h.c_str());
    curl_easy_setopt(c, CURLOPT_URL, url.c_str());
    curl_easy_setopt(c, CURLOPT_HTTPHEADER, hs);
    curl_easy_setopt(c, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(c, CURLOPT_WRITEFUNCTION, wr);
    curl_easy_setopt(c, CURLOPT_WRITEDATA, &out);
    curl_easy_setopt(c, CURLOPT_TIMEOUT, 15L);
    auto rc = curl_easy_perform(c);
    curl_slist_free_all(hs);
    curl_easy_cleanup(c);
    if(rc != CURLE_OK) throw std::runtime_error("POST failed");
    return out;
}