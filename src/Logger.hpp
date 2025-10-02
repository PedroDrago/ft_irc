#pragma once

#include <cstdlib>
#include <string>
#include <iostream>
#include <string>

#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define RESET "\033[0m"
class Logger {
	private:
	protected:
	public:
		template<typename T> static void log(const T &msg){
			std::cout << msg << std::endl;
		}
		template<typename T> static void debug(const T &msg){
			std::cout << BLUE << "[DEBUG] " << msg << RESET << std::endl;
		}
		template<typename T> static void info(const T &msg){
			std::cout << "[INFO] " << msg << std::endl;
		}
		template<typename T> static void warning(const T &msg){
			std::cout << YELLOW << "[WARNING] " << msg << RESET << std::endl;
		}
		template<typename T> static void error(const T &msg){
			std::cerr << RED << "[ERROR] " << msg << RESET << std::endl;
		}
		template<typename T> static void success(const T &msg){
			std::cout << GREEN << "[SUCCESS] " << msg << RESET << std::endl;
		}
		template<typename T> static void panic(const T &msg, int status){ 
			std::cerr << RED << "[PANIC] " << msg << RESET << std::endl;
			exit(status);
		}
};

