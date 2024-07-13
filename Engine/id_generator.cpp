#include "id_generator.h"

using namespace dx11;


int IdGenerator::GetId() { return entity_id_++; };
void IdGenerator::Reset() { entity_id_ = 0; };
