#include <sstream>
#include <string>
#include <vector>
template<typename T> std::string numToString(T value){
    std::ostringstream oss;
    oss << value;
    return oss.str();
}


std::vector<std::string> split(const std::string &str, const std::string &delimiter);
std::vector<std::string> split_by_whitespace(const std::string &str);
