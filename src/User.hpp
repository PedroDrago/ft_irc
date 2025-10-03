#ifndef USER_CPP
#define USER_CPP
#include <string>

enum state{
	PASS,
	NICK,
	USER,
	AUTH
};
class User {
private:
protected:
public:
	User();
	User(const User &src);
	User &operator = (const User &src);
	~User();
	std::string nickname;
	std::string username;
	std::string realname;
	int fd;
	bool is_in_channel;
	std::string channel;
	int random_n;
	state stt;
};
#endif
