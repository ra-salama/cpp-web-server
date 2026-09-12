#include "HttpRequest.h"
#include <sstream>
#include <algorithm>

HttpRequest parseHttpRequest(const std::string& rawRequest){
    HttpRequest request;
    std::istringstream stream(rawRequest); //raw text to stream
    std::string line;

    //extracting request line
    if(std::getline(stream,line)){
        if(!line.empty() && line.back() == '\r'){
            line.pop_back(); //removes \r
        }

        std::istringstream lineStream(line);
        if(lineStream >> request.method >> request.path >> request.version){
            request.isValid = true;
        } else {
            request.isValid = false;
            return request;
        }
    } else {
        request.isValid = false;
        return request;
    }

    //parsing Headers into key-value pairs
    while (std::getline(stream,line)){
        if(!line.empty() && line.back() == '\r'){
            line.pop_back();
        }
        if (line.empty()){
            break; //end of header and start of body
        }
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos){
            std::string headerName = line.substr(0,colonPos);
            std::string headerValue = line.substr(colonPos +1);

            size_t valueStart = headerValue.find_first_not_of(" \t");//removed whitespaces
            if (valueStart != std::string::npos){
                headerValue = headerValue.substr(valueStart);
            } else {
                headerValue = "";
            }
            request.headers[headerName] = headerValue;
        }
    }
// capturing the request body
    std::string bodyContent;
    char ch;
    while(stream.get(ch)){
        bodyContent.push_back(ch);
    }
    request.body = bodyContent;
    return request;
}