#ifndef SERVER_H
#define SERVER_H

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <unordered_map>
#include <functional>
#include "ThreadPool.h"
#include "HttpRequest.h"

using RouteHandler = std::function<std::string(const HttpRequest&)>;

class Server{
    public: 
        explicit Server(int port); //constructor, to listen to port
        ~Server(); //destructor

        bool init(); //start server socket
        void run(); //start running the requests

        void route(const std::string& method, const std::string& path, RouteHandler handler);

    private:
        int m_port; //port no.
        int m_server_fd; //socket file descriptor
        sockaddr_in m_address{}; //struct for ip addr and port 
        ThreadPool m_threadPool;
        std::unordered_map<std::string, RouteHandler> m_routes;
        void handleClient(int client_fd); //helper function to process a connection
};

#endif