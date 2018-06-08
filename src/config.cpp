//
// Created by Sixzeroo on 2018/6/8.
//

#include <fstream>
#include <sstream>

#include "config.h"

int get_config_map(const char *file_name, std::map<std::string, std::string> &configs) {
    std::ifstream fs(file_name);
    if(!fs.is_open())
    {
        return -1;
    }

    while (fs.good())
    {
        std::string line;
        std::getline(fs, line);

        if(line[0] == '#')
            continue;

        std::stringstream ss;
        ss << line;
        std::string key, value;
        std::getline(ss, key, '=');
        std::getline(ss, value, '=');

        configs[key] = value;
    }

    fs.close();
    return 0;

}