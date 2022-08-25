//
// Created by Hao Zhou on 22/08/2022.
//

#ifndef WALKAROUNDMP_WORLD_H
#define WALKAROUNDMP_WORLD_H

#include <vector>
#include <sstream>
#include <optional>
#include "constants.h"

struct Thing {
    Thing();
    Thing(int sprite_id, int id, float x, float y, float w = thing_size, float h = thing_size);
    Thing clone() const;
    void copy_from(const Thing &another_thing);

    int sprite_id, id;
    float x, y, w, h;
    float speed = default_thing_speed;
};

class World {
public:
    World();

    std::vector<Thing> &get_things() {
        return things;
    }

    Thing &add_thing(const Thing &thing);
    std::shared_ptr<std::stringstream> serialize() const;
    Thing *get_thing_by_id(int id);

private:
    std::vector<Thing> things;
    int id_counter;
};

#endif //WALKAROUNDMP_WORLD_H
