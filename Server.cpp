#include "Server.h"
#include <sstream>

//constructor
Server::Server(int port) : m_port(port), m_server_fd(-1){}

//destructor
Server::~Server(){
    if (m_server_fd != -1){
        close(m_server_fd);
        std::cout <<"[Server] Socket closed. \n";
    }
}

bool Server::init(){
    m_server_fd = socket(AF_INET, SOCK_STREAM, 0); //Ipv4 TCP
    if(m_server_fd == -1){
        std::cerr <<"[Error] Failed to create socket. \n";
        return false;
    }

    //allows reuse of recently freed addresses
    int opt=1;
    if(setsockopt(m_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))<0){
        std::cerr << "[Error] setsocketopt failed. \n";
        return false;
    }

    m_address.sin_family = AF_INET;
    m_address.sin_addr.s_addr = INADDR_ANY; //0.0.0.0
    m_address.sin_port = htons(m_port); //converts little to big-endian

    //bind socket to port
    if(bind(m_server_fd,reinterpret_cast<sockaddr*>(&m_address),sizeof(m_address))<0){
        std::cerr<<"[Error] Bind failed on port " <<m_port <<".\n";
        return false;
    }

    //to queue incoming TCP handshake connection requests. Max 10
    if(listen(m_server_fd, 10)<0){
        std::cerr <<"[Error] Listen failed. \n";
        return false;
    }

    std::cout <<"[Server] Initialized on http://localhost:" <<m_port<< "\n";
    return true;
}

void Server::run(){
    while(true){
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);

        // wait for incoming TCP connection
        int client_fd = accept(m_server_fd, reinterpret_cast<sockaddr*>(&client_addr),&client_len);
        if(client_fd < 0){
            std::cerr<<"[Error] Failed to accept client connection.\n";
            continue;
        }

        //pass the connected client socket to helper function
        handleClient(client_fd);
    }
}

void Server::handleClient(int client_fd){
    char buffer[4096] = {0};

    //stores raw HTTP text bytes in buffer array
    ssize_t bytes_read = recv(client_fd, buffer, sizeof(buffer)-1,0);

    if(bytes_read>0){
        std::cout <<"\n--- Recieved Request ---\n" <<buffer <<"------------------------\n";

        std::string body = "<html><body><h1>Modular C++ Web Server</h1></body></html>";
        std::ostringstream response_stream; 
        response_stream <<"HTTP/1.1 200 OK\r\n"
                        << "Content-Type: text/html\r\n" 
                        << "Content-Length: " <<body.length() << "\r\n" 
                        << "Connection: close\r\n\r\n" 
                        << body;
        std::string response = response_stream.str();
        
        //writes HTTP response across network back on client's socket
        send(client_fd,response.c_str(),response.length(),0);
    }
    //closes the client's dedicated connection channel
    close(client_fd);
}