#include <iostream>
#include <string>
#include <curl/curl.h>

// Callback function to write the response data to a string
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void torConnect(std::string proxy) {
    std::string readBuffer;

    // Initialize libcurl
    curl_global_init(CURL_GLOBAL_DEFAULT);
    CURL *curl = curl_easy_init();

    if (curl) {
        // Set the URL to fetch
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        // Set the SOCKS5 proxy with provided host and port
        curl_easy_setopt(curl, CURLOPT_PROXY, proxy.c_str());

        // Set up a callback to capture the response
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        // Perform the request
        CURLcode res = curl_easy_perform(curl);

        // Check for errors
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        } else {
            // Print the response
            std::cout << "Response received: " << readBuffer << std::endl;
        }

        // Clean up
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <URL> <ProxyHost> <ProxyPort>" << std::endl;
        return 1;
    }

    std::string url = argv[1];
    std::string proxyHost = argv[2];
    std::string proxyPort = argv[3];
    std::string proxy = "socks5h://" + proxyHost + ":" + proxyPort;

    torConnect(proxy);
    
    return 0;
}
// g++ -o myprogram main.cpp -lcurl
// ./myprogram http://check.torproject.org 127.0.0.1 9050
