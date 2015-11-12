#include "MemoryMap.hpp"
#include "FileMap.hpp"



struct Header {
    char reserved1[4];
    char reserved2[4];
    struct {
        uint8_t main;
        uint8_t revision;
        uint16_t release;
    } version;
    uint32_t counter;

    inline void set() {
        memcpy(reserved1, "DPDB", 4);
        memcpy(reserved2, "cntr", 4);
        version = {
            .main = 0,
            .revision = 1,
            .release = 1
        };
        counter = 0;
    }
    inline const bool check() {
        if (memcmp(reserved1, "DPDB", 4)) {
            return false;
        }
        return true;
    }
};


struct Model {
    uint64_t id;
    uint32_t type_id;
    char name[20];
    char description[96];

    inline Model() {
        memset(this, 0, sizeof(Model));
    }
    inline Model(const Model& source) {
        memcpy(this, &source, sizeof(Model));
    }
    inline Model(const uint64_t& _id, const uint32_t& _type_id, const char* _name, const char* _description) {
        id = _id;
        type_id = _type_id;
        strncpy(name, _name, sizeof(name));
        strncpy(description, _description, sizeof(description));
    }
    inline void show() {
        debug("<Model id=%lu type_id=%u name=%.*s>", id, type_id, (int) sizeof(name), name);
    }
};


int main(int argc, char const *argv[]) {

    start();

    message("basic tests on memory blocks"); {
        notice("create, clone");
        MemoryBlock<Model, uint16_t, 4096> block1;
        block1.show();
        MemoryBlock<Model, uint16_t, 4096> block2(block1);
        block2.show();
        notice("insert");
        for (int i=0; i<8; i++) {
            int index = rand() % block2.capacity;
            block2[index] = Model(i, i % 4, ":)", "this is a test");
            block2[index].show();
        }
        notice("retrieve");
        block2.show();
        for (int i=0; i<8; i++) {
            block2[rand() % block2.capacity].show();
        }
    }

    message("basic tests on memory maps"); {
        // uint64_t n = 256 * 256 * 256;
        uint64_t n = 256 * 256;
        notice("insert %lu models", n)
        MemoryMap<Model, uint16_t, 4096, 256> map;
        for (int i=0; i<n; i++) {
            map[i] = Model(i, i % 4, ":)", "this is a test");
        }
        debug("here it is!");
    }

    message("basic tests on file maps"); {
        uint64_t m = 16;
        uint64_t n = 16 * 256 * 256;
        notice("insert %lu models", n);
            FileMap<Header, Model, uint32_t, 4096, 16> map("storage/test_maps");
            debug("reserve space");
            map.reserve(n);
            debug("insert");
            for (int i=0; i<n; i++) {
                map[i] = Model(i, i % 4, ":)", "this is a test");
            }
        notice("show %lu models", m);
            for (int i=0; i<m; i++) {
                map[rand() % n].show();
            }
        notice("query %lu models randomly", n);
            int sum = 0;
            for (int i=0; i<n; i++) {
                sum += map[rand() % n].id;
            }
            debug("checksum = %u", sum);
        notice("query %lu models sequentially", n);
            for (int i=0; i<n; i++) {
                sum += map[i].id;
            }
            debug("checksum = %u", sum);
    }

    finish(return);
}
