#include "Server.hpp"
#include "Logger.hpp"
#include "Message.hpp"
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
// - colocar todas as classes em forma ortodoxa canonica
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
	// TODO: refact to use Message class
	std::vector<std::string> splited_buffer = split_by_whitespace(buffer);
	if (splited_buffer[0] == "PASS"){
		if (splited_buffer[1] != this->password){
			std::string msg = "Invalid password";
			this->send_message(user->fd, msg);
			return;
		}
		user->is_authenticated = true;
		return;
	}
	if (splited_buffer[0] == "NICK"){
		//TODO: validate nickname characters
		std::map<int, User*>::iterator it;
		for (it = this->fd_users.begin(); it != this->fd_users.end(); ++it) {
			if(it->second->nickname == splited_buffer[1]){
				std::string msg = "Nickname " + splited_buffer[1] + " is already in use";
				this->send_message(user->fd, msg);
				return;
			}
		}
		user->nickname = splited_buffer[1];
		return;
	}
	if (splited_buffer[0] == "USER"){
		// TODO: validate user characters and existence
		std::string msg = "User registered.";
		this->send_message(user->fd, msg);
		this->nick_users[user->nickname] = user;
		user->prefix = user->nickname + "!" + user->username + "@" + "<hostname>";
		user->is_registered = true;
		return;
	}
	this->send_message(user->fd, "You have not registered");
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

void Server::send_message(int target_fd, std::string message){
	// TODO: substituir pelo addr do server
	message = "<SERVER_HOST> " + message + "\r\n";
	send(target_fd, message.c_str(), message.size(), 0);
}

void Server::proccess_message(char *buffer, User *user){
	const std::string str_msg = std::string(buffer);
	Message msg(str_msg);
	if (msg.command == "PRIVMSG"){
		User *target_usr = this->get_user_by_nickname(msg.params[0]);
		if (!target_usr){
			std::string error_msg = "user with nickname " + msg.params[0] + " not found";
			this->send_message(user->fd, error_msg);
			return;
		}
		user->send_message_to_user(target_usr, msg.params[1]);
		return;
	} 
	this->send_message(user->fd, "command not implemented");
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
						buffer[bytes_recv] = '\0';
						User *current_user = this->fd_users.at(current_pollfd.fd);
						if (current_user->is_registered){
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
