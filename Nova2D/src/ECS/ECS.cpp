#include "ECS.h"

int Entity::GetId() const {
	return id;
}

void System::AddEntityToSystem(Entity entity) {

}

void System::RemoveEntityFromSystem(Entity entity) {

}

std::vector<Entity> System::GetSystemEntities() const {
	return entities;
}

const Signature System::GetComponentSignature() const {
	return componentSignature; 
}