#include <lexer.hpp>

#include <stdexcept>
#include <cstdint>

static bool is_text_char(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_';
}

static bool is_text(const std::string& str) {
    for (const char& c : str)
        if (!is_text_char(c)) return false;
    return true;
}

static void trim(std::string& str) {
    size_t l = str.find_first_not_of(" ");

    if (l == std::string::npos) {
        str.clear();
        return;
    }

    size_t r = str.find_last_not_of(" ");
    str = str.substr(l, r - l + 1);
}

std::vector<std::string> calc::get_tokens(std::string& line) {
    std::vector<std::string> tokens;

    trim(line);
    if (line.empty()) return {};

    size_t sep = line.find('=');
    size_t quot = line.find('"');
    
    if (sep == std::string::npos || (quot != std::string::npos && quot < sep)) {
        size_t sp = line.find(' ');
        if (sp == std::string::npos) throw std::runtime_error("Unknown syntax in line '" + line + "'.");

        std::string first = line.substr(0, sp);
        trim(first);
        if (!is_text(first)) throw std::runtime_error("Unknown syntax in line '" + line + "'.");

        std::string second = line.substr(sp + 1);
        trim(second);

        size_t end;
        std::string value;

        if (second[0] == '"') {
            end = second.find('"', 1);
            if (end == std::string::npos) throw std::runtime_error("Unknown syntax in line '" + line + "'.");

            value.push_back(' ');
            value += std::move(second.substr(1, end - 1));
            if (value.length() - 1 == 0) throw std::runtime_error("Empty string in line '" + line + "'.");
            value[0] = static_cast<unsigned char>(static_cast<uint8_t>(value.length()-1));
        } else {
            end = second.find(' ');

            value.push_back('\0');

            std::string tmp;

            if (end == std::string::npos) tmp = second;
            else tmp = second.substr(0, end);

            if (!is_text(tmp)) throw std::runtime_error("Invalid name format '" + tmp + "' in line '" + line + "'.");

            value += std::move(tmp);
        }

        tokens.push_back(std::move(first));
        tokens.push_back(std::move(value));

        if (end != std::string::npos) {
            std::string flag = second.substr(end + 1);
            trim(flag);
            if (!flag.empty()) {
                if (!is_text(flag)) throw std::runtime_error("Unknown syntax in line '" + line + "'.");
                tokens.push_back(std::move(flag));
            }
        }
    } else {
        std::string name = line.substr(0, sep);
        trim(name);
            
        if (name.empty()) throw std::runtime_error("No name in line '" + line + "'.");
        if (!is_text(name)) throw std::runtime_error("Invalid name format '" + name + "'.");

        tokens.push_back(std::move(name));
        tokens.push_back("=");

        std::string value = line.substr(sep + 1);
        trim(value);
        
        if (value.empty()) throw std::runtime_error("No value in line '" + line + "'.");

        size_t op = value.find_first_of("+-*/", 1);
        if (op == std::string::npos) {
            try {
                size_t pos;
                std::stof(value, &pos);
                if (pos != value.length())
                    if (!is_text(value)) throw std::runtime_error("invalid value '" + value + "'.");
            } catch (const std::invalid_argument&) {
                if (!is_text(value)) throw std::runtime_error("invalid value '" + value + "'.");
            }

            tokens.push_back(std::move(value));
        } else {
            std::string first = value.substr(0, op);
            trim(first);

            try {
                size_t pos;
                std::stof(first, &pos);
                if (pos != first.length())
                    if (!is_text(first)) throw std::runtime_error("invalid first value '" + first + "'.");
            } catch (const std::invalid_argument&) {
                if (!is_text(first)) throw std::runtime_error("invalid first value '" + first + "'.");
            }

            tokens.push_back(std::move(first));

            tokens.emplace_back(1, value[op]);
            
            std::string second = value.substr(op + 1);
            trim(second);

            try {
                size_t pos;
                std::stof(second, &pos);
                if (pos != second.length())
                    if (!is_text(second)) throw std::runtime_error("invalid second value '" + second + "'.");
            } catch (const std::invalid_argument&) {
                if (!is_text(second)) throw std::runtime_error("invalid second value '" + second + "'.");
            }

            tokens.push_back(std::move(second));
        }
    }

    return tokens;
}
