#ifndef PATROLCOMPONENT_H
#define PATROLCOMPONENT_H

// Keeps an entity moving back and forth inside a rectangle (used by enemy tanks/trucks
// so they stay on land and never drive into the sea).
struct PatrolComponent {
    float minX;
    float maxX;
    float minY;
    float maxY;

    PatrolComponent(float minX = -100000.0f, float maxX = 100000.0f, float minY = -100000.0f, float maxY = 100000.0f) {
        this->minX = minX;
        this->maxX = maxX;
        this->minY = minY;
        this->maxY = maxY;
    }
};

#endif
