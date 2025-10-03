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

#include <cstdlib> // For rand() and srand()
#include <ctime>   // For time()
#include <iostream>

#include <fstream>
#include <iostream>
#include <string>
#include <dirent.h>
#include <sys/stat.h>

#include <sys/epoll.h>
#include <fcntl.h>
#include <utility>
#include <vector>
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
	fcntl(this->sock, F_SETFL, O_NONBLOCK);

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

int Server::accept_connection(){
	srand(time(0));
	struct sockaddr_in client_addr;
	socklen_t client_addr_len = sizeof(client_addr);
	int client_socket = accept(this->sock, (struct sockaddr*)&client_addr, &client_addr_len);
	if (client_socket == -1){
		Logger::error("accept() Failed");
		return -1;
	}
	fcntl(client_socket, F_SETFL, O_NONBLOCK);
	pollfd client_pollfd;
	client_pollfd.fd = client_socket;
	client_pollfd.events = POLLIN;
	this->poll_fds.push_back(client_pollfd);
	Logger::info("new connection: fd " + numToString(client_socket));
	return client_pollfd.fd;;
}

void authenticate_user(char *buffer, User *user, Server &server){
	Logger::info("starting to authenticate user in state: " + numToString(user->stt) + "with fd: " + numToString(user->fd));
	std::vector<std::string> splited_buffer = split_by_whitespace(buffer);
	if (user->stt == PASS){
		if (splited_buffer.size() != 2 || splited_buffer[0] != "PASS"){
			std::string msg = "PASS message in the wrong format. excpected \"PASS <PASSWORD>\"";
			send(user->fd, msg.c_str(), msg.size(), 0);
			return;
		}
		if (splited_buffer[1] != server.password){
			std::string msg = "Wrong password\n";
			Logger::info("fd: " + numToString(user->fd));
			send(user->fd, msg.c_str(), msg.size(), 0);
			return;
		}
		user->stt = NICK;
		return;
	}
	if (user->stt == NICK){
		if (splited_buffer.size() != 1 || splited_buffer[0] != "NICK"){
			std::string msg = "NICK message in the wrong format. excpected \"NICK <NICKNAME>\"";
		}
		std::map<int, User*>::iterator it;
		Logger::info("len: " + numToString(server.users.size()));
		for (it = server.users.begin(); it != server.users.end(); ++it) {
			if(it->second->nickname == splited_buffer[1]){
				Logger::info("aqui: " + numToString(user->fd));
				std::string msg = "Nickname " + splited_buffer[1] + " already taken. Please choose a unique Nickname";
				send(user->fd, msg.c_str(), msg.size(), 0);
				return;
			}
			Logger::info("aca");
		}
		user->nickname = splited_buffer[1];
		user->stt = USER;
		return;
	}
	if (user->stt == USER){
		if (splited_buffer.size() != 2 || splited_buffer[0] != "USER"){
			std::string msg = "USER message in the wrong format. excpected \"USER <USERNAME>\"";
			send(user->fd, msg.c_str(), msg.size(), 0);
			return;
		}
		user->stt = AUTH;
		return;
	}
}
void proccess_message(char *buffer, User *user){
}

void Server::run(){
	char buffer[BUFFER_SIZE];

	while (true){
		int poll_count = poll(&this->poll_fds[0], this->poll_fds.size(), -1); // FIX: add timeout?
		if (poll_count < 0) {
			Logger::error("poll() failed");
			break;
		}
		for (std::size_t i = 0; i < this->poll_fds.size(); i++){
			pollfd current_pollfd = this->poll_fds[i];
			if (current_pollfd.revents & POLLIN) {
				if (current_pollfd.fd == this->sock){
					int client_pollfd= this->accept_connection();
					Logger::info("returned fd: " + numToString(client_pollfd));
					if (client_pollfd < 0){
						continue;
					}
					User *new_user = new User;
					new_user->fd = client_pollfd;
					Logger::info("new_user.fd: " + numToString(new_user->fd));
					this->users.insert(std::make_pair(client_pollfd, new_user));
					Logger::info("retrieved in map: " + numToString(this->users.at(client_pollfd)->fd));
					// Logger::warning("fd " + numToString(client_pollfd) + " number: " + numToString(a.random_n));
				} else {
					std::memset(buffer, 0, BUFFER_SIZE);
					int bytes_recv = recv(current_pollfd.fd, buffer, BUFFER_SIZE - 1, 0);
					if (bytes_recv < 0){
						continue;
						// if (errno == EAGAIN){
						// 	Logger::warning("EAGAIN");
						// 	continue;
						// }
						// if (errno == EWOULDBLOCK){
						// 	Logger::warning("EWOULDBLOCK");
						// 	continue;
						// }
						// Logger::error("recv() failed: " + std::string(strerror(errno)));
						// close(current_pollfd.fd);
						// this->poll_fds.erase(this->poll_fds.begin() + i);
						// --i;
					} else if (bytes_recv == 0){
						Logger::warning("client disconnected: fd " + numToString(current_pollfd.fd));
						close(current_pollfd.fd);
						this->poll_fds.erase(this->poll_fds.begin() + i);
						--i;
					} else {
						buffer[bytes_recv] = '\0';
						
						User *current_user = this->users.at(current_pollfd.fd);
						Logger::info("current_pollfd: " + numToString(current_pollfd.fd));
						Logger::info("user: " + numToString(&current_user));
								
						if (current_user->stt == AUTH){
							proccess_message(buffer, current_user);
						} else {
							authenticate_user(buffer, current_user, *this);
							Logger::info("state after: " + numToString(current_user->stt));
						}

						// Logger::warning(current_user.random_n);
						// Logger::info("received from fd " + numToString(current_pollfd.fd) + " : " + buffer);
						//
						// send(current_pollfd.fd, buffer, bytes_recv, 0);
					}
				}
			}
		}
	}
}
