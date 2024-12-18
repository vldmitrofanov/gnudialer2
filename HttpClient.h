#include <iostream>
#include <string>
#include <curl/curl.h>

#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

class HttpClient {
public:
    HttpClient(const std::string &host, int port, const std::string &user, const std::string &pass)
        : _host(host), _port(port), _user(user), _pass(pass) {}

    std::string get(const std::string &endpoint) {
        std::string url = "http://" + _host + ":" + std::to_string(_port) + endpoint;
        if (url.find('?') != std::string::npos) {
            url += "&api_key=" + _user + ":" + _pass;
        } else {
            url += "?api_key=" + _user + ":" + _pass;
        }
        return performRequest(url, "GET");
    }

    std::string post(const std::string &endpoint, const std::string &data) {
        std::string url = "http://" + _host + ":" + std::to_string(_port) + endpoint;
        if (url.find('?') != std::string::npos) {
            url += "&api_key=" + _user + ":" + _pass;
        } else {
            url += "?api_key=" + _user + ":" + _pass;
        }
        return performRequest(url, "POST", data);
    }

private:
    std::string _host;
    int _port;
    std::string _user;
    std::string _pass;

    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }

    std::string performRequest(const std::string &url, const std::string &method, const std::string &postData = "") {
        CURL *curl;
        CURLcode res;
        std::string response;
        std::cout << "[DEBUG](HttpClient.h) URL: " << url << std::endl;

        curl = curl_easy_init();
        if(curl) {
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
            curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);

            if (method == "POST") {
                curl_easy_setopt(curl, CURLOPT_POST, 1L);
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
            }

            res = curl_easy_perform(curl);
            if(res != CURLE_OK) {
                std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            } else {
                long http_code = 0;
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
                if (http_code >= 400) {
                    std::cerr << "HTTP error: " << http_code << std::endl;
                }
            }

            curl_easy_cleanup(curl);
        }
        return response;
    }
};

#endif
