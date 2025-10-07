#include "User.hpp"
#include "Logger.hpp"
#include "utils.hpp"
#include <sys/socket.h>
      
User::User(){
	Logger::warning("user default constructor");
	this->is_authenticated = false;
	this->is_registered = false;
};
User::User(const User &src){
	Logger::warning("user copy constructor");
	this->random_n = src.random_n;
	this->username = src.username;
	this->channel = src.channel;
	this->is_in_channel = src.is_in_channel;
	this->nickname = src.nickname;
	this->realname = src.realname;
}
User &User::operator=(const User &src){
	Logger::warning("user = operator");
	this->random_n = src.random_n;
	this->username = src.username;
	this->channel = src.channel;
	this->is_in_channel = src.is_in_channel;
	this->nickname = src.nickname;
	this->realname = src.realname;
	return *this;
}
User::~User(){
	Logger::warning("user destructor");
};


void User::send_message_to_user(User *target_usr, std::string message){
	std::string final_msg = ":" + this->prefix + " PRIVMSG " + target_usr->nickname + " :" + message + "\r\n";
	send(target_usr->fd, final_msg.c_str(), final_msg.size(), 0);
}
