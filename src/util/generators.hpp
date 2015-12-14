#ifndef __INCLUDED__utils__generators_hpp__
#define __INCLUDED__utils__generators_hpp__


#include <string>
#include <stdlib.h>

template<int length>
inline char* generate_first_gibberish() {
    static char gibberish[length + 1];
    for (int i=0; i<length; i++) {
        gibberish[i] = 'a' + rand() % 26;
    }
    gibberish[length] = '\0';
    return gibberish;
}

template<int length>
inline const char* generate_gibberish() {
    static char* gibberish = generate_first_gibberish<length>();
    static int ending_position = 0;
    gibberish[ending_position] = 'a' + rand() % 26;
    ending_position = rand() % length;
    gibberish[ending_position] = '\0';
    return gibberish;
}

const std::string number2expression(uint32_t number) {
    static const uint32_t block_values[4] = {1000000000, 1000000, 1000, 1};
    static const std::string block_names[4] = {" billion", " million", " thousand", ""};
    static const std::string unities[10] = {"zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine"};
    static const std::string specials[10] = {"ten", "eleven", "twelve", "thirteen", "forteen", "fifteen", "sixteen", "seventeen", "eighteen", "nineteen"};
    static const std::string tens[10] = {"", "", "twenty", "thirty", "forty", "fifty", "sixty", "seventy", "eighty", "ninety"};
    std::string expression;
    for (uint32_t b=0; b<4; b++) {
        uint16_t block = (number / block_values[b]) % 1000;
        if (block == 0) {
            continue;
        }
        uint8_t n0 = block % 10;
        block /= 10;
        uint8_t n1 = block % 10;
        block /= 10;
        uint8_t n2 = block % 10;
        if (n2) {
            if (!expression.empty()) {
                expression += ' ';
            }
            expression += unities[n2] + " hundred";
        }
        if (n1) {
            if (!expression.empty()) {
                expression += ' ';
            }
            if (n1 == 1) {
                expression += specials[n0];
            } else {
                expression += tens[n1];
            }
        }
        if (n0 && n1 != 1) {
            if (n1) {
                expression += '-';
            } else if (!expression.empty()) {
                expression += ' ';
            }
            expression += unities[n0];
        }
        expression += block_names[b];
    }
    return expression.empty() ? unities[0] : expression;
}


#endif // __INCLUDED__utils__generators_hpp__
