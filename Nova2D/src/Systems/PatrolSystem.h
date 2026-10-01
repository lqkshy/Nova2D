#ifndef PATROLSYSTEM_H
#define PATROLSYSTEM_H

#include "../ECS/ECS.h"
#include "../Components/TransformComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/PatrolComponent.h"
#include <cmath>

class PatrolSystem: public System {
    public:
        PatrolSystem() {
            RequireComponent<TransformComponent>();
            RequireComponent<RigidBodyComponent>();
            RequireComponent<PatrolComponent>();
        }

        void Update() {
            for (auto entity: GetSystemEntities()) {
                auto& transform = entity.GetComponent<TransformComponent>();
                auto& rigidbody = entity.GetComponent<RigidBodyComponent>();
                const auto& patrol = entity.GetComponent<PatrolComponent>();

                if (transform.position.x < patrol.minX) {
                    transform.position.x = patrol.minX;
                    rigidbody.velocity.x = std::abs(rigidbody.velocity.x);
                } else if (transform.position.x > patrol.maxX) {
                    transform.position.x = patrol.maxX;
                    rigidbody.velocity.x = -std::abs(rigidbody.velocity.x);
                }

                if (transform.position.y < patrol.minY) {
                    transform.position.y = patrol.minY;
                    rigidbody.velocity.y = std::abs(rigidbody.velocity.y);
                } else if (transform.position.y > patrol.maxY) {
                    transform.position.y = patrol.maxY;
                    rigidbody.velocity.y = -std::abs(rigidbody.velocity.y);
                }

                // The sprites face right, so mirror them when driving left
                if (entity.HasComponent<SpriteComponent>() && rigidbody.velocity.x != 0.0f) {
                    entity.GetComponent<SpriteComponent>().flip = (rigidbody.velocity.x < 0.0f) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
                }
            }
        }
};

#endif
