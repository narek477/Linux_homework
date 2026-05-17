#ifndef COLORPRINT_H
#define COLORPRINT_H

#include <iostream>
#include <string>
#include <vector>
#include <sstream>

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"

class Painter {
private:
    std::ostream& out;
    std::vector<std::string> green_words;
    std::vector<std::string> red_words;
    
    std::string color_word(const std::string& word, const std::string& color) {
        return color + word + RESET;
    }
    
    bool is_green(const std::string& word) {
        for (const auto& w : green_words) {
            if (word == w) return true;
        }
        return false;
    }
    
    bool is_red(const std::string& word) {
        for (const auto& w : red_words) {
            if (word == w) return true;
        }
        return false;
    }

public:
    Painter(std::ostream& output, 
            const std::vector<std::string>& green, 
            const std::vector<std::string>& red) 
        : out(output), green_words(green), red_words(red) {}
    
    void print(const std::string& text) {
        std::istringstream iss(text);
        std::string word;
        bool first = true;
        
        while (iss >> word) {
            if (!first) out << " ";
            first = false;
            
            if (is_green(word)) {
                out << color_word(word, GREEN);
            } else if (is_red(word)) {
                out << color_word(word, RED);
            } else if (word.find("Success") != std::string::npos) {
                out << color_word(word, GREEN);
            } else if (word.find("Error") != std::string::npos) {
                out << color_word(word, RED);
            } else if (word.find("Balance:") != std::string::npos) {
                out << color_word(word, YELLOW);
            } else {
                out << word;
            }
        }
        out << std::endl;
    }
};

#endif
