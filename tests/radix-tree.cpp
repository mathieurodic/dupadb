#include "RadixTree.hpp"


#pragma pack(1)


template <size_t size>
struct string_t {

    char data[size];

    inline operator const char* () const {
        return data;
    }

    inline const char* operator = (const string_t& source) {
        strncpy(data, source.data, size);
        return data;
    }
    inline const char* operator = (const char* source) {
        strncpy(data, source, size);
        return data;
    }
    inline const char* operator = (const std::string& source) {
        strncpy(data, source.data(), size);
        return data;
    }

    inline int operator - (const string_t<size>& other) {
        return memcmp(data, other.data, size);
    }

};

struct Model {
    uint32_t id;
    string_t<16> name;
    inline Model() {
        memset(this, 0, sizeof(*this));
    }
    inline Model(const Model& source) {
        memcpy(this, &source, sizeof(*this));
    }
    inline Model(const uint32_t _id, const char* _name) {
        id = _id;
        name = _name;
    }
    inline void show() const {
        printf("<Model id=%u name=%s>\n", id, (const char*)name);
    }
};


#ifndef offsetof
#define offsetof(C, P) (size_t)(((C*)(NULL))->P)
#endif


int main(int argc, char const *argv[]) {
    RadixTree<char[16], Model, uint16_t, offsetof(Model, name)> tree;

    tree.show();

    tree.insert(Model(1, "un"));
    // {
    //     tree.show();
    //     return 0;
    // }
    tree.insert(Model(2, "deux"));
    tree.insert(Model(3, "trois"));
    tree.insert(Model(4, "quatre"));
    tree.insert(Model(5, "cinq"));
    tree.insert(Model(6, "six"));
    tree.insert(Model(7, "sept"));
    tree.insert(Model(8, "huit"));
    tree.insert(Model(9, "neuf"));
    tree.insert(Model(10, "dix"));
    tree.insert(Model(11, "onze"));
    tree.insert(Model(12, "douze"));

    printf("\n\n\n");


    Model model;
    char key[16];
    strncpy(key, "un", 16);
    if (tree.find(key, model)) {
        model.show();
    }
    strncpy(key, "douze", 16);
    if (tree.find(key, model)) {
        model.show();
    }
    return 0;
}
