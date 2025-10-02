#ifndef CHANEL_HPP
#define CHANEL_HPP
#include <string>
#include <vector>
#include "User.hpp"
#include "Operator.hpp"
class Chanel {
private:
protected:
public:
	Chanel();
	Chanel(Chanel &src);
	Chanel &operator = (Chanel &src);
	~Chanel();
	std::string identifier;
	std::string topic;
	std::vector<User> users;
	std::vector<Operator> operators;
};
#endif
