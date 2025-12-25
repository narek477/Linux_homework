#include <iostream>
#include <string>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>


#define BUFFER_SIZE 1024

bool running = true;

void recv_thread(int sock) {
	char buffer[BUFFER_SIZE];
	while(running) {
		int len = recv(sock, buffer, BUFFER_SIZE, 0);
		if(len <= 0) {
			std::cout << "Disconnected" << std::endl;
			running = false;
			break;
		}
		std::cout << "/r" << std::string(buffer, len) << "\n>" << std::flush;
	}
}


int main() {
	int sock = socket(AF_INET, SOCK_STREAM, 0);

	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(8080);
	inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

	if(connect(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
		std::cout << "Cannot connect" << std::endl;
		return 1;
	}

	std::string name;
	std::cout << "Enter name: ";
	std::getline(std::cin, name);
	send(sock, name.c_str(), name.length(), 0);

	std::thread reciever(recv_thread, sock);
	
	while(running) {
		std::string msg;
		std::getline(std::cin, msg);

		if(!running) break;

		if(!msg.empty()) {
			send(sock, msg.c_str(), msg.length(), 0);
			if(msg == "/exit") {
				break;
			}
		}
		std::cout << "> " << std::flush;
	}

	running = false;
	reciever.join();
	close(sock);

	return 0;	

}
