#include"entity.h"
#include "id_generator.h"

using namespace Engine;

IEntity::IEntity() { entity_id_ = IdGenerator::GetId(); }

IEntity::~IEntity() {}

int IEntity::GetEntityId() const { return entity_id_; }