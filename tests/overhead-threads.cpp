#include "util/logging.hpp"
#include "util/generators.hpp"

#include <string>

#include <pthread.h>
#include <unistd.h>


static std::string debugstring = "";

void* callback(void* name){
    usleep(100000);
    if (!debugstring.empty()) {
        debugstring += ", ";
    }
    debugstring += (char*) name;
    pthread_exit(name);
}


int main(int argc, char const *argv[]) {
    start();
    static const uint32_t n_threads = 16;
    std::string names[n_threads];
    pthread_t threads[n_threads];

    message("creating %u strings", n_threads);
    for (uint32_t t=0; t<n_threads; t++) {
        names[t] = number2expression(t);
        debug("%s", names[t].c_str());
    }

    message("starting %u threads", n_threads);
    for (uint32_t t=0; t<n_threads; t++) {
        const char* name = names[t].c_str();
        notice("starting thread #%u", t);
        int rc = pthread_create(threads + t, NULL, callback, (void*)name);
        if (rc) {
            error("return code is %d for thread {%s}", rc, name);
        }
        debug("%s", debugstring.c_str());
    }

    message("applying action to %u threads", n_threads);
    for (uint32_t t=0; t<n_threads; t++) {
        char* name = "";
        int rc = pthread_join(threads[t], (void**) &name);
        // int rc = pthread_detach(threads[t]);
        if (rc) {
            error("return code is %d for thread {%s}", rc, name);
        } else {
            notice("#%u {%s}", t, name);
        }
        debug("%s", debugstring.c_str());
    }

    finish(return);
}
