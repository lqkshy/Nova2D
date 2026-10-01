#ifndef KEYBOARDCONTROLLEDCOMPONENT_H
#define KEYBOARDCONTROLLEDCOMPONENT_H

#include <glm/glm.hpp>

struct KeyboardControlledComponent {
    glm::vec2 upVelocity;
    glm::vec2 rightVelocity;
    glm::vec2 downVelocity;
    glm::vec2 leftVelocity;
    glm::vec2 facing;   // last direction the entity moved in (used to aim the bullets)

    KeyboardControlledComponent(glm::vec2 upVelocity = glm::vec2(0), glm::vec2 rightVelocity = glm::vec2(0), glm::vec2 downVelocity = glm::vec2(0), glm::vec2 leftVelocity = glm::vec2(0)) {
        this->upVelocity = upVelocity;
        this->rightVelocity = rightVelocity;
        this->downVelocity = downVelocity;
        this->leftVelocity = leftVelocity;
        this->facing = glm::vec2(0.0f, -1.0f);
    }
};

#endif
