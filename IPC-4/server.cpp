#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <map>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#define PORT 8080
#define BUFFER_SIZE 1024

std::mutex mtx;
std::vector<int> clients;
std::vector<std::string> usernames;

void broadcast(const std::string& msg, int sender) {
	mtx.lock();
	for(int i = 0; i < clients.size(); i++) {
		if(clients[i] != sender) {
			send(clients[i], msg.c_str(), msg.length(), 0);	
		}
	}
	mtx.unlock();
}


void handle(int sock) {
	char buffer[BUFFER_SIZE];

	int len = recv(sock, buffer, BUFFER_SIZE, 0);
	if(len <= 0) {
		close(sock);
		return;
	}
	std::string name(buffer, len);

	mtx.lock();
	clients.push_back(sock);
	usernames.push_back(name);
	mtx.unlock();

	std::string join_msg = name + " joined";
	std::cout << join_msg << std::endl;
	broadcast(join_msg, sock);


	while(true) {
		len = recv(sock, buffer, BUFFER_SIZE, 0);
		if(len <= 0) { break; }

		std::string msg(buffer, len);

		if(msg == "/exit") {
			broadcast(name + "left", sock);
			break;
		}
		else if(msg == "/list") {
			std::string list = "Users: ";
		        mtx.lock();
			for(auto& u: usernames) {
				list += u + " ";
			}
			mtx.unlock();
			send(sock, list.c_str(), list.length(), 0);	
		}
		else {
			broadcast(name + ": " + msg, sock);
		}
	}

	mtx.lock();
	for(int i = 0; i < clients.size(); i++) {
		if(clients[i] == sock) {
			clients.erase(clients.begin() + i);
			usernames.erase(usernames.begin() + i);
			break;
		}
	}

	mtx.unlock();

	close(sock);

}


int main() {
	int server_fd = socket(AF_INET, SOCK_STREAM, 0);

	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	addr.sin_addr.s_addr = INADDR_ANY;

	bind(server_fd, (sockaddr*)&addr, sizeof(addr));
	listen(server_fd, 5);

	std::cout << "Server started on port" << PORT << std::endl;

	while(true) {
		int client = accept(server_fd, nullptr, nullptr);
		std::thread(handle, client).detach();
	}

}








