#include "Server.hpp"
#include "Logger.hpp"
#include "User.hpp"
#include "utils.hpp"
#include <cstdio>
#include <cstdlib>
#include <locale>
#include <netinet/in.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <csignal>
#include <cstdio>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <cstddef>
#include <string>
#include <cstdlib> 
#include <ctime>   
#include <string>
#include <dirent.h>
#include <sys/stat.h>

#include <sys/epoll.h>
#include <fcntl.h>
#include <utility>
#include <vector>
// TODO: 
// - resolver os FIX que anotei
// - classes de channels
// - comandos de channels
// - broadcast de mensagens em channels
// - classes de operators
// - comandos de operators
// - subject menciona que temos que lidar com mensgens tipo "com^Dman^Dd".
//        - "In order to process a command, you have to first aggregate the received packets in order to rebuild it."
//        - não entendi qual o problema de uma mensagem assim, mas blz

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
	Logger::info("new connection accepted");
	return client_pollfd.fd;;
}

void Server::authenticate_user(char *buffer, User *user){
	std::vector<std::string> splited_buffer = split_by_whitespace(buffer);
	if (user->stt == PASS){
		if (splited_buffer.size() != 2 || splited_buffer[0] != "PASS"){
			std::string msg = "PASS message in the wrong format. excpected \"PASS <PASSWORD>\"\n";
			send(user->fd, msg.c_str(), msg.size(), 0);
			return;
		}
		if (splited_buffer[1] != this->password){
			std::string msg = "Wrong password\n";
			send(user->fd, msg.c_str(), msg.size(), 0);
			return;
		}
		user->stt = NICK;
		return;
	}
	if (user->stt == NICK){
		if (splited_buffer.size() != 1 || splited_buffer[0] != "NICK"){
			std::string msg = "NICK message in the wrong format. excpected \"NICK <NICKNAME>\"\n";
		}
		std::map<int, User*>::iterator it;
		for (it = this->fd_users.begin(); it != this->fd_users.end(); ++it) {
			if(it->second->nickname == splited_buffer[1]){
				std::string msg = "Nickname " + splited_buffer[1] + " already taken. Please choose a unique Nickname\n";
				send(user->fd, msg.c_str(), msg.size(), 0);
				return;
			}
		}
		user->nickname = splited_buffer[1];
		user->stt = USER;
		return;
	}
	if (user->stt == USER){
		if (splited_buffer.size() != 2 || splited_buffer[0] != "USER"){
			std::string msg = "USER message in the wrong format. excpected \"USER <USERNAME>\"\n";
			send(user->fd, msg.c_str(), msg.size(), 0);
			return;
		}
		std::string msg = "User registered.\n";
		send(user->fd, msg.c_str(), msg.size(), 0);
		user->stt = AUTH;
		this->nick_users[user->nickname] = user;
		return;
	}
}

User* Server::get_user_by_fd(int fd){
	try {
		User *usr = this->fd_users.at(fd);
		return usr;
	} catch (std::out_of_range &e) {
		return NULL;
	}
}
User* Server::get_user_by_nickname(std::string &nickname){
	try {
		User *usr = this->nick_users.at(nickname);
		return usr;
	} catch (std::out_of_range &e) {
		return NULL;
	}
}

void Server::proccess_message(char *buffer, User *user){
	std::vector<std::string> splited_buffer = split_by_whitespace(buffer);
	if (splited_buffer[0] == "PRIVMSG"){
		Logger::warning("priv detected");
		// FIX: add validations.
		std::string &target_nick = splited_buffer[1];
		std::map<int, User*>::iterator it;
		int target_fd = -1;
		for (it = this->fd_users.begin(); it != this->fd_users.end(); ++it) {
			if(it->second->nickname == target_nick && it->second->stt == AUTH){
				Logger::warning("User found");
				target_fd = it->second->fd;
				// FIX: essa logica de mensagem tira todos os espacos. Preciso entender se a parada de `:` é o jeito
				// certo de delimitar as mensagem sempre ou se preciso fazer um parsing mais minucioso.
				std::string target_msg = user->nickname + "!" + user->username + " PRIVMSG " + target_nick + ":";
				for (std::size_t i = 2; i < splited_buffer.size(); i++) { 
					target_msg += splited_buffer[i];
				}
				target_msg += "\n";
				send(target_fd, target_msg.c_str(), target_msg.size(), 0);
				return;
			}
			Logger::warning("User NOT found");
			std::string error_msg = "401 " + user->nickname + " " + target_nick + " :No such nick/channel\n";
			Logger::warning("send");
			send(user->fd, error_msg.c_str(), error_msg.size(), 0);
			Logger::warning("after");
			return;
		}
	}
	std::string msg = "Command not supported | internal server error: " + std::string(buffer) + "\n";
	send(user->fd, msg.c_str(), msg.size(), 0);
	return;
}

void Server::run(){
	char buffer[BUFFER_SIZE];

	while (true){
		int poll_count = poll(&this->poll_fds[0], this->poll_fds.size(), -1); // FIX: adicionar timeout?
		if (poll_count < 0) {
			Logger::error("poll() failed");
			break;
		}
		for (std::size_t i = 0; i < this->poll_fds.size(); i++){
			pollfd current_pollfd = this->poll_fds[i];
			if (current_pollfd.revents & POLLIN) {
				if (current_pollfd.fd == this->sock){
					int client_pollfd= this->accept_connection();
					if (client_pollfd < 0){
						Logger::warning("continued on accept_connection");
						continue;
					}
					User *new_user = new User;
					new_user->fd = client_pollfd;
					this->fd_users.insert(std::make_pair(client_pollfd, new_user));
				} else {
					std::memset(buffer, 0, BUFFER_SIZE);
					int bytes_recv = recv(current_pollfd.fd, buffer, BUFFER_SIZE - 1, 0);
					if (bytes_recv < 0){
						Logger::warning("continued on recv");
						continue;
						// FIX: acho que isso ta errado, mas pelo que entendi do subject a gnt n
						// pode usar o errno, n ta nem na lista de coisas permitidas (na webserv tava só
						// que tinha uma regra de quando não pode usar). Mas se puder o ideal aqui é checar
						// por EAGAIN ou EWOULDBLOCK
					} else if (bytes_recv == 0){
						Logger::warning("client disconnected: fd " + numToString(current_pollfd.fd));
						if (this->fd_users.find(current_pollfd.fd) != this->fd_users.end()){
							User *current_user = get_user_by_fd(current_pollfd.fd);
							if (current_user){
								std::string current_nickname = current_user->nickname;
								delete current_user;
								current_user = NULL;
								this->fd_users.erase(current_pollfd.fd);
								this->nick_users.erase(current_nickname);
							}
						}
						close(current_pollfd.fd);
						this->poll_fds.erase(this->poll_fds.begin() + i);
						--i;
					} else {
						Logger::warning("Got in else");
						buffer[bytes_recv] = '\0';
						User *current_user = this->fd_users.at(current_pollfd.fd);
						Logger::warning("user state: " + numToString(current_user->stt));
						if (current_user->stt == AUTH){
							this->proccess_message(buffer, current_user);
						} else {
							authenticate_user(buffer, current_user);
						}
					}
				}
			}
		}
	}
}
