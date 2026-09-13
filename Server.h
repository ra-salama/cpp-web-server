#pragma once
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include "ThreadPool.h"

class Server{
    public: 
        explicit Server(int port); //constructor, to listen to port
        ~Server(); //destructor

        bool init(); //start server socket
        void run(); //start running the requests

    private:
        int m_port; //port no.
        int m_server_fd; //socket file descriptor
        sockaddr_in m_address{}; //struct for ip addr and port 
        ThreadPool m_threadPool;

        void handleClient(int client_fd); //helper function to process a connection
};