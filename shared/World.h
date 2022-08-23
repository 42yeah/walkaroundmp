//
// Created by Hao Zhou on 22/08/2022.
//

#ifndef WALKAROUNDMP_WORLD_H
#define WALKAROUNDMP_WORLD_H

#include <vector>
#include "constants.h"

struct Thing {
    Thing();
    Thing(int id, float x, float y, float w = thing_size, float h = thing_size);
    Thing clone() const;

    int id;
    float x, y, w, h;
    float speed = default_thing_speed;
};

class World {
public:
    std::vector<Thing> &get_things() {
        return things;
    }

    void add_thing(const Thing &thing);

private:
    std::vector<Thing> things;
};

#endif //WALKAROUNDMP_WORLD_H
