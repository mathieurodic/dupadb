#include "util/logging.hpp"
#include "util/generators.hpp"
#include "util/types.hpp"

#include "Counter.hpp"
#include "BTree.hpp"

#include <stdint.h>
#include <stdlib.h>
#include <string>

#pragma pack(1)


struct Entity {
    struct DB;

    uint16_t id;
    char type;
    str_t<16> name;
    str_t<256> description;

    inline void show() {
        debug("<Entity id=%-3hu type=%c name=%-4s description=`%s`>", id, type, name.data(), description.data());
    }
};

struct Entity::DB {
    Counter<Entity, uint32_t, 4096, 256> primary;
    BTree<uint32_t, std::tuple<char, str_t<16>>> btree__type__name;
    BTree<uint32_t, std::tuple<str_t<16>, char>> btree__name__type;
    BTree<uint32_t, str_t<256>> btree__description;
    inline DB(std::string path) :
        primary((path + ".primary").c_str(), 1024*1024),
        btree__type__name((path + ".btree.type+name").c_str()),
        btree__name__type((path + ".btree.name+type").c_str()),
        btree__description((path + ".btree.description").c_str()) {}
    inline bool add(Entity& entity) {
        // primary index
        uint32_t id = primary.append(entity);
        if (id == 0) {
            return false;
        }
        entity.id = id;
        // btree index: (type, name)
        btree__type__name.insert(
            * (std::tuple<char, str_t<16>>*) &(entity.type),
            id
        );
        // btree index: (name, type)
        std::tuple<str_t<16>, char> btree__name__type__key(entity.name, entity.type);
        btree__name__type.insert(
            btree__name__type__key,
            id
        );
        // btree index: (description)
        btree__description.insert(
            entity.description,
            id
        );
        return true;
    }
};

struct DB {
    Entity::DB entities;

    inline DB(std::string path) :
        entities(path + "/entities") {}

    inline bool add(Entity& entity) {
        return entities.add(entity);
    }
};


int main(int argc, char const *argv[]) {
    start();

    message("initialize database");
    mkdir("storage/test_3", 0777);
    DB db("storage/test_3");

    uint64_t n = 1000;
    message("insert %lu entities", n);
    for (uint64_t i=0; i<n; i++) {
        Entity entity;
        uint16_t value = rand() % 100;
        entity.type = 'a' + i % 26;
        sprintf(entity.name._data, "#%u", value);
        entity.description = number2expression(value);

        if (!db.add(entity)) {
            error("could not insert entity:");
        }
        entity.show();
    }

    message("browse entities by index: type,name") {
        auto& index = db.entities.btree__type__name;
        for (auto it=index.begin(); it!=index.end(); ++it) {
            size_t id = it.value();
            db.entities.primary.get(id).show();
        }
    }

    message("browse entities by index: name,type") {
        auto& index = db.entities.btree__name__type;
        for (auto it=index.begin(); it!=index.end(); ++it) {
            size_t id = it.value();
            db.entities.primary.get(id).show();
        }
    }

    message("browse entities by index: description") {
        auto& index = db.entities.btree__description;
        for (auto it=index.begin(); it!=index.end(); ++it) {
            size_t id = it.value();
            db.entities.primary.get(id).show();
        }
    }

    finish(return);
}
