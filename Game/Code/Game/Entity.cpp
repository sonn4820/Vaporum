#include "Game/Entity.hpp"

Entity::Entity(Game* owner)
	:m_game(owner)
{
}

Entity::~Entity()
{
}

Mat44 Entity::GetModeMatrix() const
{
	Mat44 modelMat = Mat44();
	modelMat = m_orientation.GetAsMatrix_IFwd_JLeft_KUp();
	modelMat.SetTranslation3D(m_position);
	return modelMat;
}
