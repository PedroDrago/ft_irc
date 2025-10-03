#include "User.hpp"
#include "Logger.hpp"
#include "utils.hpp"
      
User::User(){
	Logger::warning("this is happening");
	this->stt = PASS;
};
User::User(const User &src){
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
	this->random_n = src.random_n;
	this->username = src.username;
	this->channel = src.channel;
	this->is_in_channel = src.is_in_channel;
	this->nickname = src.nickname;
	this->realname = src.realname;
	Logger::warning("equal is happening");
	this->stt = src.stt;
	return *this;
}
User::~User(){
	Logger::error("user destructor");
};
