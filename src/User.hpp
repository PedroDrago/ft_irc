#ifndef USER_CPP
#define USER_CPP
#include <string>

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
	std::string prefix;
	bool is_registered;
	bool is_authenticated;
	int fd;
	bool is_in_channel;
	std::string channel;
	int random_n;
	void send_message_to_user(User *target_usr, std::string message);
};
#endif
