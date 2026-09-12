#include "Server.h"
#include <iostream>
#include <sstream>
#include "HttpRequest.h"
#include <unistd.h>
#include <sys/socket.h>
#include <fstream>
#include <sstream>
                                                                                              
std::string readFileAsString(const std::string& filePath){
    std::ifstream file(filePath, std::ios::in | std::ios::binary);
    if(!file.is_open()){
        return "";
    }
    std::ostringstream contents;
    contents << file.rdbuf(); //streams entire file contents into memory in a single IO Opperation
    return contents.str(); //buffer stream to standard string 
}

//extracts file extension and returns official MIME String
std::string getMimeType(const std::string& filePath){
    size_t dotPos = filePath.find_last_of('.');
    if (dotPos == std::string::npos){ //default binary fallback
        return "application/octet-stream";
    }
    std::string ext = filePath.substr(dotPos);
    if (ext == ".html" || ext == "htm") return "text/html";
    if (ext == ".css")                  return "text/css";
    if (ext == ".js")                  return "application/javascript";
    if (ext == ".json")                  return "application/json";
    if (ext == ".png")                  return "image/png";
    if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
    if (ext == ".gif")                  return "image/gif";
    if (ext == ".svg")                  return "image/svg+xml";
    if (ext == ".ico")                  return "image/x-icon";
    if (ext == ".txt")                  return "text/plain";

    return "application/octet-stream";
}

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

        std::string rawRequest(buffer, bytes_read);
        HttpRequest request = parseHttpRequest(rawRequest);

        std::string response;
            //invalid HTTP Request
            if(!request.isValid){
            std::string body = "400 Bad Request";
            response = "HTTP/1.1 400 Bad Request\r\n"
                        "Content-Type: text/plain\r\n"
                        "Content-Length: " + std::to_string(body.length()) + "\r\n"
                        "Connection: close\r\n\r\n" + body;

        /* } else if (request.path == "/"){ 
            std::string body = readFileAsString("../public/index.html");

            if(body.empty()){
                std::string errorBody = "500 Internal Server Error: Could not load file";
                response = "HTTP/1.1 500 Internal Server Error\r\n"
                        "Content-Type: text/plain\r\n"
                        "Content-Length: " + std::to_string(errorBody.length()) + "\r\n"
                       "Connection: close\r\n\r\n" + errorBody;
            } else {
                response = "HTTP/1.1 200 Ok\r\n"
                        "Content-Type: text/html\r\n"
                        "Content-Length: " + std::to_string(body.length()) + "\r\n"
                       "Connection: close\r\n\r\n" + body;
            } */
        }
        // Security Check: Block Directory Traversal before touching the filesystem
        else if (request.path.find("..") != std::string::npos || request.path.find('\0') != std::string::npos) {
            std::string errorBody = "403 Forbidden: Invalid Path";
            response = "HTTP/1.1 403 Forbidden\r\n"
                       "Content-Type: text/plain\r\n"
                       "Content-Length: " + std::to_string(errorBody.length()) + "\r\n"
                       "Connection: close\r\n\r\n" + errorBody;
        
        // API Route
        }   else if(request.path == "/json"){
            std::string body = "{\"message\": \"Hello from API\", \"status\" : \"success\" }";
            response = "HTTP/1.1 200 OK\r\n"
                        "Content-Type: application/json\r\n"
                        "Content-Length: " + std::to_string(body.length()) + "\r\n"
                       "Connection: close\r\n"
                       "\r\n" + body;
         } else {
            // Static File Serving
            std :: string reqPath = request.path;

            if(reqPath == "/"){ //standard page to load
                reqPath = "/index.html";
            }

            std::string filePath = "../public" + reqPath;
            std::string body = readFileAsString(filePath);

            if (body.empty()){
            std::string errorBody = "404 Page Not Found";
            response = "HTTP/1.1 404 Not Found\r\n"
                        "Content-Type: text/plain\r\n"
                        "Content-Length: " + std::to_string(body.length()) + "\r\n"
                       "Connection: close\r\n"
                       "\r\n" + body; 
            } else {
                std::string mimeType = getMimeType(filePath);
                response = "HTTP/1.1 200  OK\r\n"
                        "Content-Type: " + mimeType + "\r\n"
                        "Content-Length: " + std::to_string(body.length()) + "\r\n"
                       "Connection: close\r\n"
                       "\r\n" + body; 

            
            }
        }
        //writes HTTP response across network back on client's socket
        send(client_fd, response.c_str(), response.length(), 0);
    }
    // closes client socket connection
    close(client_fd);
}
