#include "BaseModel.h"

void BaseModel::DrawModel() {
	m_texture.activate();
	m_shader.activate();
	m_shader.setUniformMat4("modelTransform", m_modelTransform);
	glBindVertexArray(m_VAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}