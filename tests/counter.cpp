#include "Counter.hpp"
#include "util/generators.hpp"


struct Model {
    uint32_t id;
    uint16_t type_id;
    char name[26];
    char description[224];

    inline Model(const uint16_t& _type_id, const char* _name, const char* _description) {
        id = 0;
        type_id = _type_id;
        strncpy(name, _name, sizeof(name));
        strncpy(description, _description, sizeof(description));
    }

    inline void show() const {
        debug("<Model id=%u type_id=%hu name=`%.*s`>", id, type_id, (int) sizeof(name), name);
    }

};



int main(int argc, char const *argv[]) {

    start();

    message("test counter") {

        uint32_t id;
        uint32_t n = 16 * 65536;

        notice("instanciate counter")
            Counter<Model, uint32_t, true, 0, 16*1024*1024, 4096, 16> counter("storage/test_indices");

        notice("append an instance") {
            Model instance(2, "hello world", "this is nothing but a simple test...");
            instance.show();
            id = counter.insert(instance);
            debug("inserted with id = %u", id);
            instance.show();
        }

        notice("retrieve the instance") {
            counter[id].show();
        }

        notice("insert %u elements with reservation", n) {
            debug("reserve");
            counter.reserve(n);
            debug("insert");
            for (uint32_t i=0; i<n; i++) {
                Model instance(i & 8, generate_gibberish<26>(), generate_gibberish<224>());
                id = counter.insert(instance);
                // instance.show();
            }
        notice("insert %u elements without reservation", n);
            debug("insert");
            for (uint32_t i=0; i<n; i++) {
                Model instance(i & 8, generate_gibberish<26>(), generate_gibberish<224>());
                id = counter.insert(instance);
                // instance.show();
            }
        }
    }

    finish(return);
}
