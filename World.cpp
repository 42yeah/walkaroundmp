//
// Created by Hao Zhou on 22/08/2022.
//

#include "World.h"

Thing::Thing() : id(0), x(0.0f), y(0.0f), w(thing_size), h(thing_size) {}

Thing::Thing(int id, float x, float y, float w, float h) : id(id), x(x), y(y), w(w), h(h) {}

Thing Thing::clone() const {
    return Thing(id, x, y);
}

void World::add_thing(const Thing &thing) {
    things.push_back(thing);
}
