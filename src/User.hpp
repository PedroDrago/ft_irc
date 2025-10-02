#ifndef USER_CPP
#define USER_CPP
#include <string>
class User {
private:
protected:
public:
	User();
	User(User &src);
	User &operator = (User &src);
	~User();
	std::string nickname;
	std::string username;
	std::string realname;
	bool authenticated;
	bool is_in_channel;
	std::string channel;
};
#endif
