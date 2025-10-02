#include <cstdlib>
#include <string>
#include "Server.hpp"
#include "Logger.hpp"
#include "utils.hpp"

bool is_number(const std::string& s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

bool validate_argv(char *argv[]){
	bool ret = true;
	if (!is_number(argv[1])){
		ret = false;
		Logger::error("server port must be a number, got " + std::string(argv[1]));
	}
	if (std::string(argv[2]).empty()){
		ret = false;
		Logger::error("server password must not be empty");
	}
	return ret;
}

int main(int argc, char *argv[]) {
	
	if (argc != 3){
		Logger::panic(std::string(argv[0]) + " expected 3 arguments, got " + numToString(argc), 1);
	}
	if (!validate_argv(argv)){
		exit(1);
	}
	Server server(argv[1], argv[2]);
	server.init();
	server.run();
}
