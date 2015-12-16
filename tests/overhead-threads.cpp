#include "util/logging.hpp"
#include "util/generators.hpp"

#include <string>
#include <pthread.h>


static std::string debugstring = "";

void* callback(void* name){
    if (!debugstring.empty()) {
        debugstring += ", ";
    }
    debugstring += (char*) name;
    pthread_exit(NULL);
}


int main(int argc, char const *argv[]) {
    start();
    static const uint32_t n_threads = 16;
    pthread_t threads[n_threads];

    message("creating %u threads", n_threads);
    for(uint32_t t=0; t<n_threads; t++) {
        const char* name = number2expression(t).c_str();
        notice("In main: creating thread %u", t);
        int rc = pthread_create(&threads[t], NULL, callback, (void*)name);
        if (rc) {
            error("return code from pthread_create() is %d for thread {%s}", rc, name);
        }
        debug("%s", debugstring.c_str());
    }

    finish(return);
}
