#include <sstream>
#include <string>
template<typename T> std::string numToString(T value){
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

