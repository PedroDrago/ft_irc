#ifndef SERVER_CPP
#define SERVER_CPP
#include <map>
#include <netinet/in.h>
#include <string>
#include "User.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <poll.h>

#define MAX_CLIENTS 100
#define BUFFER_SIZE 512
class Server {
private:
protected:
public:
	Server(std::string port, std::string password);
	Server(Server &src);
	Server &operator = (Server &src);
	~Server();
	std::string password;
	int port;
	std::string port_str;
	std::vector<pollfd> poll_fds;
	struct pollfd server_pollfd;
	std::map<int, User*> users_fd;
	std::map<std::string, User*> users_nick;
	int sock;
	void init();
	int accept_connection();
	void authenticate_user(char *buffer, User *user);
	void proccess_message(char *buffer, User *user);
	void run();
	User *get_user_by_nickname(std::string &nickname);
	User *get_user_by_fd(int fd);

};
#endif
