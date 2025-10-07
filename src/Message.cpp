#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include "Message.hpp"

static void trimCRLF(std::string &line) {
    while (!line.empty() && (line[line.size()-1] == '\n' || line[line.size()-1] == '\r'))
        line.erase(line.size()-1);
}


Message::Message(const std::string &raw) {
    std::string line = raw;
    trimCRLF(line);

    if (!line.empty() && line[0] == ':') {
        size_t spacePos = line.find(' ');
        if (spacePos != std::string::npos) {
            this->prefix = line.substr(1, spacePos - 1);
            line = line.substr(spacePos + 1);
        } else {
            this->prefix = line.substr(1);
            line = "";
        }
    }

    size_t spacePos = line.find(' ');
    if (spacePos != std::string::npos) {
        this->command = line.substr(0, spacePos);
        line = line.substr(spacePos + 1);
    } else {
        this->command = line;
        line = "";
    }

    while (!line.empty()) {
        if (line[0] == ':') {
            this->params.push_back(line.substr(1));
            break;
        }

        spacePos = line.find(' ');
        if (spacePos != std::string::npos) {
            this->params.push_back(line.substr(0, spacePos));
            line = line.substr(spacePos + 1);

            while (!line.empty() && line[0] == ' ')
                line.erase(0,1);
        } else {
            this->params.push_back(line);
            break;
        }
    }
}
Message::~Message(){};
