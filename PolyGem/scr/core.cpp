#include "core.h"
#include "core_scene.h"
#include "core_functions.h"

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

void plg::Vec2::RotateIP(float angle) {
	float rx = std::cosf(angle);
	float ry = std::sinf(angle);
	x = x * rx - y * ry;
	y = x * ry + y * rx;
}

void plg::Vec2::RotateIP(float angle, const Vec2& centroid) {
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

void plg::Vec2::RotateByVecIP(const Vec2& normal) {
	x = x * normal.x - y * normal.y;
	y = x * normal.y + y * normal.x;
}

void plg::Vec2::RotateByVecIP(const Vec2& normal, const Vec2& centroid) {
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

void plg::Edge::Rotate(float angle) {
	Vec2 normal(std::cosf(angle), std::sinf(angle));
	m_VertexMesh->operator[](m_End).RotateByVecIP(normal, m_VertexMesh->operator[](m_End));
}

void plg::Edge::Rotate(Vec2 normal) {
	m_VertexMesh->operator[](m_End).RotateByVecIP(normal, m_VertexMesh->operator[](m_End));
}

void plg::Edge::RotateByCenter(float angle) {
	Vec2 center((m_VertexMesh->operator[](m_Start) + m_VertexMesh->operator[](m_End)) / 2);
	Vec2 normal(std::cosf(angle), std::sinf(angle));
	m_VertexMesh->operator[](m_Start).RotateByVecIP(normal, center);
	m_VertexMesh->operator[](m_End).RotateByVecIP(normal, center);
}

void plg::Edge::RotateByCenter(Vec2 normal) {
	Vec2 center((m_VertexMesh->operator[](m_Start) + m_VertexMesh->operator[](m_End)) / 2);
	m_VertexMesh->operator[](m_Start).RotateByVecIP(normal, center);
	m_VertexMesh->operator[](m_End).RotateByVecIP(normal, center);
}

void plg::Edge::RotateByCentroid(float angle, Vec2 centroid) {
	Vec2 normal(std::cosf(angle), std::sinf(angle));
	m_VertexMesh->operator[](m_Start).RotateByVecIP(normal, centroid);
	m_VertexMesh->operator[](m_End).RotateByVecIP(normal, centroid);
}

void plg::Edge::RotateByCentroid(Vec2 normal, Vec2 centroid) {
	m_VertexMesh->operator[](m_Start).RotateByVecIP(normal, centroid);
	m_VertexMesh->operator[](m_End).RotateByVecIP(normal, centroid);
}

void plg::Edge::Move(Vec2 offset) {
	m_VertexMesh->operator[](m_Start).AddVec(offset);
	m_VertexMesh->operator[](m_End).AddVec(offset);
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

plg::Mesh::Mesh(std::initializer_list<plg::Vertex> vertices) : m_Vertices(vertices), m_Mode(MeshMode::PLG_VERTEX) {
	for (size_t i = 0; i < m_Vertices.GetCapacity(); i++)
		m_Edges.Append(Edge(i, (i + 1) % m_Vertices.GetCapacity(), &m_Vertices));
}
plg::Mesh::Mesh(const Mesh& other){}
plg::Mesh::Mesh(Mesh&& other){}
plg::Mesh& plg::Mesh::operator=(const Mesh& other){

	return *this;
}
plg::Mesh& plg::Mesh::operator=(Mesh&& other){

	return *this;
}
void plg::Mesh::AddVertex(plg::Vertex object){
	m_Vertices.Append(object);
}
void plg::Mesh::AddEdge(plg::Vertex object){}
void plg::Mesh::AddFace(plg::Face object){}

void plg::Mesh::Render(SDL_Renderer* renderer, Vec2 offset) {
	for (auto it_vertex = m_Vertices.Begin(); it_vertex < it_vertex.end_ptr && m_Mode == MeshMode::PLG_VERTEX; it_vertex++)
		drawCircle(renderer, *it_vertex, 2, { 216, 216, 216, SDL_ALPHA_OPAQUE });
	
	SDL_SetRenderDrawColor(renderer, 76, 156, 216, SDL_ALPHA_OPAQUE);
	for (auto it_edge = m_Edges.Begin(); it_edge < it_edge.end_ptr; it_edge++)
		SDL_RenderDrawLine(renderer, (int)it_edge->GetStart().x, (int)it_edge->GetStart().y, (int)it_edge->GetEnd().x, (int)it_edge->GetEnd().y);
}
