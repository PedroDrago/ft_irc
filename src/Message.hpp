#ifndef MESSAGE_HPP
#define MESSAGE_HPP
#include <string>
#include <vector>
class Message {
private:
protected:
public:
	Message(const std::string &raw);
	Message(Message &src);
	Message &operator = (Message &src);
	~Message();
    std::string prefix;
    std::string command;
    std::vector<std::string> params;
};
#endif
