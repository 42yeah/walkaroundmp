//
// Created by Hao Zhou on 22/08/2022.
//

#include "World.h"

Thing::Thing() : id(0), x(0.0f), y(0.0f), w(thing_size), h(thing_size) {}

Thing::Thing(int sprite_id, int id, float x, float y, float w, float h) : sprite_id(sprite_id), id(id), x(x), y(y), w(w), h(h) {}

Thing Thing::clone() const {
    return Thing(sprite_id, id, x, y, w, h);
}

void Thing::copy_from(const Thing &another_thing) {
    sprite_id = another_thing.sprite_id;
    id = another_thing.id;
    x = another_thing.x;
    y = another_thing.y;
    w = another_thing.w;
    h = another_thing.h;
}

Thing &World::add_thing(const Thing &thing) {
    things.push_back(thing);
    id_counter++;
    Thing &ret = things[things.size() - 1];
    ret.id = id_counter;
    return ret;
}

std::shared_ptr<std::stringstream> World::serialize() const {
    std::shared_ptr<std::stringstream> ret = std::make_shared<std::stringstream>();
    std::stringstream &ss = *ret;
    int things_size = things.size();
    ss.write((char *) &things_size, sizeof(int));
    for (const auto &thing : things) {
        ss.write((char *) &thing, sizeof(Thing));
    }
    return ret;
}

Thing *World::get_thing_by_id(int id) {
    auto it = std::find_if(things.begin(), things.end(), [id](const Thing &thing) {
        return thing.id == id;
    });
    if (it == things.end()) {
        return nullptr;
    }
    return &(*it);
}

World::World() : id_counter(0) {

}
