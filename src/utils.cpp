#include <string>
#include <vector>
std::vector<std::string> split(const std::string &str, const std::string &delimiter) {
	std::vector<std::string> result;
	std::string::size_type start = 0;
	std::string::size_type end = str.find(delimiter);

	while (end != std::string::npos) {
		result.push_back(str.substr(start, end - start));
		start = end + delimiter.length();
		end = str.find(delimiter, start);
	}
	result.push_back(str.substr(start));
	return result;
}
