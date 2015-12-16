#include "util/logging.hpp"
#include "util/generators.hpp"
#include "util/types.hpp"

#include "Counter.hpp"
#include "BTree.hpp"

#include <stdint.h>
#include <stdlib.h>
#include <string>

#pragma pack(1)

template<typename column_t, typename model1_t, typename model2_t=model1_t>
struct Filter {
    enum op_t {LT, LTE, EQ, GTE, GT};
    op_t _op;
    std::size_t _offset1;
    std::size_t _offset2;
    column_t _value;
    bool is_value;
    inline Filter(op_t op, std::size_t offset, column_t value) : _op(op), _offset1(offset), _value(value), is_value(true) {}
    inline Filter(op_t op, std::size_t offset1, std::size_t offset2) : _op(op), _offset1(offset1), _offset2(offset2), is_value(false) {}
};
// template<typename model1_t, typename model2_t, typename column_t>
// struct Filter {
//     enum op_t {LT, LTE, EQ, GTE, GT};
//     op_t _op;
//     std::size_t _offset;
//     column_t _value;
//     inline Filter(op_t op, std::size_t offset, column_t value) : _op(op), _offset(offset), _value(value) {}
// };


template<typename model1_t, typename column_t>
struct Column {
    std::size_t _offset;
    inline Column(std::size_t offset) : _offset(offset) {}
    // comparison with other value
    inline Filter<column_t, model1_t> operator < (column_t value) {
        return Filter<column_t, model1_t>(Filter<column_t, model1_t>::op_t::EQ, _offset);
    }
    inline Filter<column_t, model1_t> operator <= (column_t value) {
        return Filter<column_t, model1_t>(Filter<column_t, model1_t>::op_t::EQ, _offset);
    }
    inline Filter<column_t, model1_t> operator == (column_t value) {
        return Filter<column_t, model1_t>(Filter<column_t, model1_t>::op_t::EQ, _offset);
    }
    inline Filter<column_t, model1_t> operator >= (column_t value) {
        return Filter<column_t, model1_t>(Filter<column_t, model1_t>::op_t::EQ, _offset);
    }
    inline Filter<column_t, model1_t> operator > (column_t value) {
        return Filter<column_t, model1_t>(Filter<column_t, model1_t>::op_t::EQ, _offset);
    }
    // comparison with other value
    template<typename model2_t>
    inline Filter<column_t, model1_t, model2_t> operator == (Column<model2_t, column_t>& other_column) {
        return Filter<column_t, model1_t, model2_t>(Filter<column_t, model1_t, model2_t>::op_t::EQ, _offset, other_column._offset);
    }
};


//
//
// Structure-specific
//
//

struct EntityType {
    // properties
    uint8_t id;
    str_t<32> name;
    // debugging
    inline void show() {
        debug("<EntityType id=%-3hhu name=%s>", id, name.data());
    }

    // database
    struct DB;
};


struct EntityType::DB {
    // indices
    Counter<EntityType, uint32_t, 4096, 256> primary;
    BTree<uint32_t, str_t<32>> btree__name;

    // constructor
    inline DB(std::string path) :
        primary((path + ".primary").c_str(), 1024*1024),
        btree__name((path + ".btree.name").c_str()) {}
    // add an element to all indices
    inline bool add(EntityType& entity_type) {
        // primary index
        uint32_t id = primary.append(entity_type);
        if (id == 0) {
            return false;
        }
        entity_type.id = id;
        // btree index: (name)
        btree__name.insert(
            entity_type.name,
            id
        );
        return true;
    }

    // for querying purpose
    static Column<EntityType, uint8_t> id;
    static Column<EntityType, str_t<16>> name;
};
Column<EntityType, uint8_t> EntityType::DB::id(offsetof(EntityType, id));
Column<EntityType, str_t<16>> EntityType::DB::name(offsetof(EntityType, name));


struct Entity {
    // properties
    uint16_t id;
    uint8_t type_id;
    str_t<16> name;
    str_t<256> description;
    // debugging
    inline void show() {
        debug("<Entity id=%-3hu type_id=%-3hhu name=%-4s description=`%s`>", id, type_id, name.data(), description.data());
    }

    // database
    struct DB;
};

struct Entity::DB {
    // indices
    Counter<Entity, uint32_t, 4096, 256> primary;
    BTree<uint32_t, std::tuple<char, str_t<16>>> btree__type_id__name;
    BTree<uint32_t, std::tuple<str_t<16>, char>> btree__name__type_id;
    BTree<uint32_t, str_t<256>> btree__description;

    // constructor
    inline DB(std::string path) :
        primary((path + ".primary").c_str(), 1024*1024),
        btree__type_id__name((path + ".btree.type_id+name").c_str()),
        btree__name__type_id((path + ".btree.name+type_id").c_str()),
        btree__description((path + ".btree.description").c_str()) {}
    // add an element to all indices
    inline bool add(Entity& entity) {
        // primary index
        uint32_t id = primary.append(entity);
        if (id == 0) {
            return false;
        }
        entity.id = id;
        // btree index: (type_id, name)
        btree__type_id__name.insert(
            * (std::tuple<char, str_t<16>>*) &(entity.type_id),
            id
        );
        // btree index: (name, type_id)
        std::tuple<str_t<16>, char> btree__name__type_id__key(entity.name, entity.type_id);
        btree__name__type_id.insert(
            btree__name__type_id__key,
            id
        );
        // btree index: (description)
        btree__description.insert(
            entity.description,
            id
        );
        return true;
    }

    // for querying purpose
    static Column<Entity, uint16_t> id;
    static Column<Entity, uint8_t> type_id;
    static Column<Entity, str_t<16>> name;
    static Column<Entity, str_t<256>> description;
};
Column<Entity, uint16_t> Entity::DB::id(offsetof(Entity, id));
Column<Entity, uint8_t> Entity::DB::type_id(offsetof(Entity, type_id));
Column<Entity, str_t<16>> Entity::DB::name(offsetof(Entity, name));
Column<Entity, str_t<256>> Entity::DB::description(offsetof(Entity, description));



struct DB {
    Entity::DB entities;
    EntityType::DB entity_types;

    inline DB(std::string path) :
        entities(path + "/entities"),
        entity_types(path + "/entity_types") {}

    // add new elements
    inline bool add(Entity& entity) {
        return entities.add(entity);
    }
    inline bool add(EntityType& entity_type) {
        return entity_types.add(entity_type);
    }

    // querying
    template<typename... columns>
    struct Query {
        DB& _db;
        std::tuple<columns...> _columns_values;
        // constructor
        inline Query(DB& db) : _db(db) {}
        // selecting
        template<typename table_t>
        inline Query<columns...>& from() {
            return *this;
        }
        // // joining
        // template<typename model_t>
        // inline Query<columns...>& join(const Filter<model_t, column_t>& condition) {
        //     return *this;
        // }
        // filtering
        template<typename column_t, typename model_t>
        inline Query<columns...>& filter(const Filter<model_t, column_t> condition) {
            return *this;
        }
        //
    };
    template<typename... columns>
    Query<columns...> select() {
        return Query<columns...>(*this);
    }
};


int main(int argc, char const *argv[]) {
    start();

    message("initialize database");
    mkdir("storage/test_3", 0777);
    DB db("storage/test_3");

    uint64_t n = 10;
    message("insert %lu entities", n);
    for (uint64_t i=0; i<n; i++) {
        Entity entity;
        uint16_t value = rand() % 100;
        entity.type_id = 'a' + i % 26;
        sprintf(entity.name._data, "#%u", value);
        entity.description = number2expression(value);

        if (!db.add(entity)) {
            error("could not insert entity:");
        }
        entity.show();
    }

    message("browse entities by index: type_id,name") {
        auto& index = db.entities.btree__type_id__name;
        for (auto it=index.begin(); it!=index.end(); ++it) {
            size_t id = it.value();
            db.entities.primary.get(id).show();
        }
    }

    message("browse entities by index: name,type_id") {
        auto& index = db.entities.btree__name__type_id;
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

    message("query with ORM") {
        auto query = db
          .select<Entity>()
        //   .join<EntityType>(Entity::DB::type_id == EntityType::DB::id)
        ;
    }

    finish(return);
}
