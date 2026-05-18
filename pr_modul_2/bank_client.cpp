#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdlib>
#include "bank_common.h"
#include "colorprint.h"

class BankClient {
private:
    int sock_fd;
    bool connected;
    
public:
    BankClient() : sock_fd(-1), connected(false) {}
    
    bool connect_to_server(const std::string& host, int port) {
        sock_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (sock_fd == -1) {
            std::cerr << "Failed to create socket" << std::endl;
            return false;
        }
        
        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        
        if (inet_pton(AF_INET, host.c_str(), &server_addr.sin_addr) <= 0) {
            std::cerr << "Invalid address" << std::endl;
            return false;
        }
        
        if (connect(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
            std::cerr << "Failed to connect to server" << std::endl;
            return false;
        }
        
        connected = true;
        return true;
    }
    
    std::string send_command(const std::string& cmd) {
        if (!connected) return "Error: Not connected";
        
        send(sock_fd, cmd.c_str(), cmd.length(), 0);
        
        char buffer[4096];
        memset(buffer, 0, sizeof(buffer));
        int bytes_read = recv(sock_fd, buffer, sizeof(buffer) - 1, 0);
        
        if (bytes_read <= 0) {
            return "Error: Server disconnected";
        }
        
        return std::string(buffer);
    }
    
    void disconnect() {
        if (sock_fd != -1) {
            close(sock_fd);
            sock_fd = -1;
        }
        connected = false;
    }
    
    ~BankClient() {
        disconnect();
    }
    
    void run(const std::string& host, int port) {
        if (!connect_to_server(host, port)) {
            std::cerr << "Failed to connect to server" << std::endl;
            return;
        }
        
        std::cout << "Connected to bank server at " << host << ":" << port << std::endl;
        std::cout << "Type 'exit' to quit, 'shutdown' to stop the server" << std::endl;
        std::cout << std::endl;
        
        
        std::vector<std::string> green_words = {"Success:", "Balance:"};
        std::vector<std::string> red_words = {"Error:"};
        Painter painter(std::cout, green_words, red_words);
        
        std::string line;
        
        while (true) {
            std::cout << "bank> ";
            std::getline(std::cin, line);
            
            if (line.empty()) continue;
            
            if (line == "exit") {
                std::cout << "Goodbye!" << std::endl;
                break;
            }
            
            std::string response = send_command(line);
            painter.print(response);
            
            if (response.find("Server is shutting down") != std::string::npos) {
                break;
            }
        }
        
        disconnect();
    }
};

int main(int argc, char* argv[]) {
    std::string host = DEFAULT_HOST;
    int port = DEFAULT_PORT;
    
    if (argc > 1) {
        host = argv[1];
    }
    if (argc > 2) {
        port = std::atoi(argv[2]);
        if (port <= 0 || port > 65535) {
            std::cerr << "Invalid port number" << std::endl;
            return 1;
        }
    }
    
    BankClient client;
    client.run(host, port);
    
    return 0;
}
