#include "Server.h"
#include <iostream>

int main(){
    Server server(8080);
    //Register GET Route
    server.route("GET","/api/health",[](const HttpRequest&){
        std::string body = "{\"status\": \"UP\", \"uptime\": \"ok\"}";
        return "HTTP/1.1 200 OK\r\n"
               "Content-Type: application/json\r\n"
               "Content-Length: " + std::to_string(body.length()) + "\r\n"
               "Connection: close\r\n\r\n" + body;
    });
    // Register POST route
    server.route("POST", "/api/echo", [](const HttpRequest& req) {
        std::string body = "{\"received_body\": " + (req.body.empty() ? "\"none\"" : req.body) + "}";
        return "HTTP/1.1 200 OK\r\n"
               "Content-Type: application/json\r\n"
               "Content-Length: " + std::to_string(body.length()) + "\r\n"
               "Connection: close\r\n\r\n" + body;
    });

    if (server.init()){
        server.run();
    }
    return 0;
}