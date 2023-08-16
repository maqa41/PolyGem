#include "core.h"

void plg::Vec2::Normalize() {
	if (x == 0.0f && y == 0.0f) { return; }
	float l = sqrtf(SquareMagnitude());
	x = x / l;
	y = y / l;
}

float plg::Vec2::NormalReturnMag() {
	if (x == 0.0f && y == 0.0f) { return 0.0f; }
	float l = sqrtf(SquareMagnitude());
	x = x / l;
	y = y / l;
	return l;
}

plg::Vec2 plg::Vec2::Rotate(float angle) {
	float rx = std::cosf(angle);
	float ry = std::sinf(angle);
	return Vec2((x * rx - y * ry), (x * ry + y * rx));
}

plg::Vec2 plg::Vec2::Rotate(float angle, const Vec2& centroid) {
	float rx = std::cosf(angle);
	float ry = std::sinf(angle);
	float dx = x - centroid.x;
	float dy = y - centroid.y;
	return Vec2(x + (dx * rx - dy * ry), y + (dx * ry + dy * rx));
}

void plg::Vec2::_Rotate(float angle) {
	float rx = std::cosf(angle);
	float ry = std::sinf(angle);
	x = x * rx - y * ry;
	y = x * ry + y * rx;
}

void plg::Vec2::_Rotate(float angle, const Vec2& centroid) {
	float rx = std::cosf(angle);
	float ry = std::sinf(angle);
	float dx = x - centroid.x;
	float dy = y - centroid.y;
	x += dx * rx - dy * ry - dx;
	y += dx * ry + dy * rx - dy;
}

plg::Vec2 plg::Vec2::RotateByVec(const Vec2& normal, const Vec2& centroid) {
	float dx = x - centroid.x;
	float dy = y - centroid.y;
	return Vec2(x + dx * normal.x - dy * normal.y - dx, y + dx * normal.y + dy * normal.x - dy);
}

void plg::Vec2::_RotateByVec(const Vec2& normal, const Vec2& centroid) {
	float dx = x - centroid.x;
	float dy = y - centroid.y;
	x += dx * normal.x - dy * normal.y - dx;
	y += dx * normal.y + dy * normal.x - dy;
}

void plg::Vec2::ConjugateWith(Vec2& vec) {
	float tempX = x;
	float tempY = y;
	x = vec.x;
	y = vec.y;
	vec.x = tempX;
	vec.y = tempY;
}

void plg::Line::RotateByCenter(float angle) {
	Vec2 center((*m_Start + *m_End) / 2);
	Vec2 normal(std::cosf(angle), std::sinf(angle));
	m_Start->RotateByVec(normal, center);
	m_End->RotateByVec(normal, center);
}

void plg::Line::RotateByCenter(Vec2 normal) {
	Vec2 center((*m_Start + *m_End) / 2);
	m_Start->_RotateByVec(normal, center);
	m_End->_RotateByVec(normal, center);
}

void plg::Line::RotateByCentroid(float angle, Vec2 centroid) {
	Vec2 normal(std::cosf(angle), std::sinf(angle));
	m_Start->_RotateByVec(normal, centroid);
	m_End->_RotateByVec(normal, centroid);
}

void plg::Line::RotateByCentroid(Vec2 normal, Vec2 centroid) {
	m_Start->_RotateByVec(normal, centroid);
	m_End->_RotateByVec(normal, centroid);
}

void plg::Line::Move(Vec2 offset) {
	m_Start->AddVec(offset);
	m_End->AddVec(offset);
}

void plg::Face::Move(Vec2 offset) {
	for (auto iter = m_VertexIndices.Begin(); iter < iter.end_ptr; iter++) {
		(m_Vertices->operator[](*iter)).AddVec(offset);
	}
}

plg::Vec2 plg::Face::GetCenter() {
	Vec2 center;
	for (auto iter = m_VertexIndices.Begin(); iter < iter.end_ptr; iter++) {
		center += m_Vertices->operator[](*iter);
	}
	return center / m_VertexIndices.GetSize();
}