#include "User.hpp"
#include "Logger.hpp"
#include "utils.hpp"
      
User::User(){
	Logger::warning("user default constructor");
	this->stt = PASS;
};
User::User(const User &src){
	Logger::warning("user copy constructor");
	this->random_n = src.random_n;
	this->username = src.username;
	this->channel = src.channel;
	this->is_in_channel = src.is_in_channel;
	this->nickname = src.nickname;
	this->realname = src.realname;
	Logger::warning("copy is happening. this: " + numToString(this->stt) + " | src: " + numToString(src.stt));
	this->stt = src.stt;
}
User &User::operator=(const User &src){
	Logger::warning("user = operator");
	this->random_n = src.random_n;
	this->username = src.username;
	this->channel = src.channel;
	this->is_in_channel = src.is_in_channel;
	this->nickname = src.nickname;
	this->realname = src.realname;
	this->stt = src.stt;
	return *this;
}
User::~User(){
	Logger::warning("user destructor");
};

void User::send_privmsg(std::string target_nickname, std::string message){
}
