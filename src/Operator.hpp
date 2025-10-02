#ifndef OPERATOR_HPP
#define OPERATOR_HPP
#include "User.hpp"
class Operator : public User {
private:
protected:
public:
	Operator();
	Operator(Operator &src);
	Operator &operator = (Operator &src);
	~Operator();
};
#endif
