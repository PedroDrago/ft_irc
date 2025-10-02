#ifndef MESSAGE_HPP
#define MESSAGE_HPP
#include <string>
class Message {
private:
protected:
public:
	Message();
	Message(Message &src);
	Message &operator = (Message &src);
	~Message();
	std::string user; // NOTE: eventually a User()
	std::string channel; // NOTE: eventually a Channel
	std::string msg;
};
#endif
