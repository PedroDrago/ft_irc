#include "Server.hpp"
#include "Logger.hpp"
#include "utils.hpp"
#include <cstdio>
#include <cstdlib>
#include <netinet/in.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <csignal>
#include <cstdio>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <ostream>
#include <sys/types.h>
#include <algorithm>
#include <cstddef>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <fstream>
#include <iostream>
#include <string>
#include <dirent.h>
#include <sys/stat.h>

#include <sys/epoll.h>
#include <fcntl.h>
#define MAX_CONNECTIONS 2048 // FIX: Adjust

Server::Server(std::string port, std::string password): password(password), port_str(port){
	this->port = std::atoi(port.c_str());
};

Server::~Server(){
	for (std::size_t i = 0; i < this->poll_fds.size(); ++i) {
		close(this->poll_fds[i].fd);
	}
};

int setNonBlocking(int fd){
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
		return -1;
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
		return -1;
	return 0;
}

void Server::init(){
	this->sock = socket(AF_INET, SOCK_STREAM, 0);
	if (this->sock == -1){
		Logger::panic("socket() failed", 1);
	}

	int n = 1;
	if (setsockopt(this->sock, SOL_SOCKET, SO_REUSEADDR, &n, sizeof(n)) == -1){
		close(this->sock);
		Logger::panic("setsockopt() failed", 1);
	}

	struct sockaddr_in addr;
	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(this->port);
	addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(this->sock, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		close(this->sock);
		Logger::panic("bind() failed: ", 1);
	}

	if (listen(this->sock, SOMAXCONN) == -1) {
		close(this->sock);
		Logger::panic("listen() failed: ", 1);
	}

	pollfd poll_fd;
	poll_fd.fd = this->sock;
	poll_fd.events = POLLIN;
	this->poll_fds.push_back(poll_fd);
	Logger::info("Server listening at :" + numToString(this->port));
}

bool Server::accept_connection(){
	struct sockaddr_in client_addr;
	socklen_t client_addr_len = sizeof(client_addr);
	int client_socket = accept(this->sock, (struct sockaddr*)&client_addr, &client_addr_len);
	if (client_socket == -1){
		Logger::error("accept() Failed");
		return false;
	}

	pollfd new_poll_fd;
	new_poll_fd.fd = client_socket;
	new_poll_fd.events = POLLIN;
	this->poll_fds.push_back(new_poll_fd);
	Logger::info("new connection: fd " + numToString(client_socket));
	return true;
}

void receive_client_message(pollfd current_pollfd, char buffer[BUFFER_SIZE]){
	int bytes_recv = recv(current_pollfd.fd, buffer, BUFFER_SIZE - 1, 0);
	if (bytes_recv < 0){
		Logger::error("recv() failed");
		close(current_pollfd.fd);
		this->poll_fds.erase(this->poll_fds.begin() + i);
		--i;
	} else if (bytes_recv == 0){
		Logger::warning("client disconnected: fd " + numToString(current_pollfd.fd));
		close(current_pollfd.fd);
		this->poll_fds.erase(this->poll_fds.begin() + i);
		--i;
	} else {
		buffer[bytes_recv] = '\0';
		Logger::info("received from fd " + numToString(current_pollfd.fd) + " : " + buffer);
		send(current_pollfd.fd, buffer, bytes_recv, 0);
	}
}

void Server::run(){
	char buffer[BUFFER_SIZE];

	while (true){
		int poll_count = poll(&this->poll_fds[0], this->poll_fds.size(), -1); // FIX: add timeout?
		if (poll_count == -1) {
			Logger::error("poll() failed");
			break;
		}

		for (std::size_t i = 0; i < this->poll_fds.size(); i++){
			pollfd current_pollfd = this->poll_fds[i];
			if (current_pollfd.revents & POLLIN) {
				if (current_pollfd.fd == this->sock){
					int ret = this->accept_connection();
					if (!ret){ // NOTE: acho que é desnecessário.
						continue;
					}

				} else {
					std::memset(buffer, 0, BUFFER_SIZE);
				}
			}
		}
	}
}
