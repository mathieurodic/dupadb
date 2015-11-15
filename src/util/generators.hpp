#ifndef __INCLUDED__utils__generators_hpp__
#define __INCLUDED__utils__generators_hpp__


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


#endif // __INCLUDED__utils__generators_hpp__
