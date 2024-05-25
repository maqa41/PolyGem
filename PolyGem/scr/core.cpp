#include "core_scene.h"
#include "core_functions.h"
#include <unordered_map>

#define float_max std::numeric_limits<float>::max()
#define GRID_SIZE 64
#define G_SHIFT 6

static plg::Vec2 s_GetCircleCenter(plg::Vec2 left, plg::Vec2 middle, plg::Vec2 right) {
	plg::Vec2 line_1 = (middle - left).RotateByVec(plg::Vec2(0.0f, 1.0f));
	plg::Vec2 line_2 = (right - middle).RotateByVec(plg::Vec2(0.0f, 1.0f));
	plg::Vec2 segment_start_1 = (left + middle) * 0.5;
	plg::Vec2 segment_end_1 = segment_start_1 + line_1;
	plg::Vec2 segment_start_2 = (middle + right) * 0.5;
	plg::Vec2 segment_end_2 = segment_start_2 + line_2;

	float t1 = ((segment_start_2.y - segment_end_2.y) * (segment_start_1.x - segment_start_2.x) + 
		(segment_end_2.x - segment_start_2.x) * (segment_start_1.y - segment_start_2.y)) / 
		((segment_end_2.x - segment_start_2.x) * (segment_start_1.y - segment_end_1.y) - 
		(segment_start_1.x - segment_end_1.x) * (segment_end_2.y - segment_start_2.y));
	segment_start_1.AddScaledVec(line_1, t1);
	return segment_start_1;
}

static bool s_InsideCircumCircle(plg::Vec2 vert1, plg::Vec2 vert2, plg::Vec2 vert3, plg::Vec2 other) {
	plg::Vec2 line_1 = (vert2 - vert1).RotateByVec(plg::Vec2(0.0f, 1.0f));
	plg::Vec2 line_2 = (vert3 - vert2).RotateByVec(plg::Vec2(0.0f, 1.0f));
	plg::Vec2 segment_start_1 = (vert1 + vert2) * 0.5;
	plg::Vec2 segment_end_1 = segment_start_1 + line_1;
	plg::Vec2 segment_start_2 = (vert2 + vert3) * 0.5;
	plg::Vec2 segment_end_2 = segment_start_2 + line_2;

	float t1 = ((segment_start_2.y - segment_end_2.y) * (segment_start_1.x - segment_start_2.x) +
		(segment_end_2.x - segment_start_2.x) * (segment_start_1.y - segment_start_2.y)) /
		((segment_end_2.x - segment_start_2.x) * (segment_start_1.y - segment_end_1.y) -
			(segment_start_1.x - segment_end_1.x) * (segment_end_2.y - segment_start_2.y));
	segment_start_1.AddScaledVec(line_1, t1);
	return segment_start_1.GetDistancetoSquared(other) < segment_start_1.GetDistancetoSquared(vert1);
}

static bool s_InsideTriangle(plg::Vec2 vert1, plg::Vec2 vert2, plg::Vec2 vert3, plg::Vec2 other) {
	plg::Vec2 normal1 = (vert1 - vert2).RotateByVec(plg::Vec2(0.0f, 1.0f));
	plg::Vec2 normal2 = (vert2 - vert3).RotateByVec(plg::Vec2(0.0f, 1.0f));
	plg::Vec2 normal3 = (vert3 - vert1).RotateByVec(plg::Vec2(0.0f, 1.0f));

	plg::Vec2 vert10 = other - vert1;
	plg::Vec2 vert20 = other - vert2;
	plg::Vec2 vert30 = other - vert3;

	float S1 = vert10.ScalarProduct(normal1);
	float S2 = vert20.ScalarProduct(normal2);
	float S3 = vert30.ScalarProduct(normal3);
	float tolerance = 0.0001f;

	if ((S1 < 0 && S2 < 0 && S3 < 0) ||
		(S1 < tolerance && S2 < 0 && S3 < 0) ||
		(S1 < 0 && S2 < tolerance && S3 < 0) ||
		(S1 < 0 && S2 < 0 && S3 < tolerance)) {
		return true;
	}
	return false;
}

static bool s_CollideVertex(plg::Vertex vertex, plg::Vec2 mousePos) {
	return (vertex.GetDistancetoSquared(mousePos) < 25);
}

static bool s_CollideEdge(plg::Edge edge, container::List<plg::Vertex>* vertices, plg::Vec2 mousePos) {
	plg::Vec2 vertS = edge.GetStart(vertices);
	plg::Vec2 vertE = edge.GetEnd(vertices);
	plg::Vec2 p1 = vertS - mousePos;
	plg::Vec2 p2 = vertE - vertS;
	bool p2l = std::powf(p2.x * p1.y - p1.x * p2.y, 2) < 25 * p2.SquareMagnitude(); // check for distance from point to line
	bool p2v = std::abs(vertE.GetDistancetoSquared(mousePos) - vertS.GetDistancetoSquared(mousePos)) < vertE.GetDistancetoSquared(vertS); //check distance from point to vertices
	return p2l && p2v;
}

static bool s_CollideFace(plg::Face face, container::List<plg::Vertex>* vertices, plg::Vec2 mousePos) {
	plg::Vec2 vert1 = vertices->operator[](face.m_Vert1);
	plg::Vec2 vert2 = vertices->operator[](face.m_Vert2);
	plg::Vec2 vert3 = vertices->operator[](face.m_Vert3);
	return s_InsideTriangle(vert1, vert2, vert3, mousePos);
}

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

plg::Mesh::Mesh(std::initializer_list<plg::Vertex> vertices) : m_Vertices(vertices), m_Edges(vertices.size()) {
	Vec2 topLeft(INFINITY, INFINITY), bottomRight(-INFINITY, -INFINITY);
	size_t T_cap = m_Vertices.GetSize() + 3;
	Vec2* vertexMesh = new Vec2[T_cap];
	for (size_t i = 0; i < m_Vertices.GetCapacity(); i++) {
		if (m_Vertices[i].x < topLeft.x)
			topLeft.x = m_Vertices[i].x;
		else if (m_Vertices[i].x > bottomRight.x)
			bottomRight.x = m_Vertices[i].x;
		if (m_Vertices[i].y < topLeft.y)
			topLeft.y = m_Vertices[i].y;
		else if (m_Vertices[i].y > bottomRight.y)
			bottomRight.y = m_Vertices[i].y;
		vertexMesh[i] = m_Vertices[i];
	}
	float dx = bottomRight.x - topLeft.x;
	float dy = bottomRight.y - topLeft.y;
	float dmax = (dx > dy) ? dx : dy;
	float xmid = (bottomRight.x + topLeft.x) * 0.5f;
	float ymid = (bottomRight.y + topLeft.y) * 0.5f;

	//Add big triangle
	vertexMesh[T_cap - 3] = Vec2(xmid - 20 * dmax, ymid + 20 * dmax);
	vertexMesh[T_cap - 2] = Vec2(xmid, ymid - 20 * dmax);
	vertexMesh[T_cap - 1] = Vec2(xmid + 20 * dmax, ymid + 20 * dmax);
	container::List<Face> faces(m_Vertices.GetSize());
	faces.Append(Face(T_cap - 3, T_cap - 2, T_cap - 1));
	container::List<Edge> edgeBuffer(12);

	for (size_t vertex = 0; vertex < T_cap - 3; vertex++) {
		for (auto face = faces.Begin(); face < face.end_ptr; face++) {
			if (s_InsideCircumCircle(vertexMesh[face->m_Vert1], vertexMesh[face->m_Vert2], vertexMesh[face->m_Vert3], vertexMesh[vertex])) {
				edgeBuffer.Append(Edge(face->m_Vert1, face->m_Vert2));
				edgeBuffer.Append(Edge(face->m_Vert2, face->m_Vert3));
				edgeBuffer.Append(Edge(face->m_Vert3, face->m_Vert1));
				faces.Remove(face);
			}
		}
		for (auto edge1 = edgeBuffer.Begin(); edge1 < edge1.end_ptr; edge1++) {
			bool isBadEdge = false;
			for (auto edge2 = edge1 + 1; edge2 < edge2.end_ptr; edge2++) {
				if (*edge1 == *edge2) {
					edgeBuffer.Remove(edge2);
					isBadEdge = true;
				}
			}
			if (isBadEdge) {
				edgeBuffer.Remove(edge1);
			}
		}
		for (auto edge = edgeBuffer.Begin(); edge < edge.end_ptr; edge++) {
			faces.Append(Face(edge->m_Start, edge->m_End, vertex));
		}
		edgeBuffer.Clear();
	}
	for (auto face = faces.Begin(); face < face.end_ptr; face++) {
		if (!(T_cap - 1 == face->m_Vert1 || T_cap - 1 == face->m_Vert2 || T_cap - 1 == face->m_Vert3 ||
			T_cap - 2 == face->m_Vert1 || T_cap - 2 == face->m_Vert2 || T_cap - 2 == face->m_Vert3 ||
			T_cap - 3 == face->m_Vert1 || T_cap - 3 == face->m_Vert2 || T_cap - 3 == face->m_Vert3)) {
			m_Faces.Append(*face);
			
			Edge edge1(face->m_Vert1, face->m_Vert2), edge2(face->m_Vert2, face->m_Vert3), edge3(face->m_Vert3, face->m_Vert1);
			m_Edges.Append(edge1);
			m_Edges.Append(edge2);
			m_Edges.Append(edge3);
		}
	}
	delete[] vertexMesh;
}

plg::Mesh::Mesh(const Mesh& other)
	: m_Vertices(other.m_Vertices), m_Edges(other.m_Edges), m_Faces(other.m_Faces) { }

plg::Mesh::Mesh(Mesh&& other) noexcept
	: m_Vertices(std::move(other.m_Vertices)), m_Edges(std::move(other.m_Edges)), m_Faces(std::move(other.m_Faces)) { }

plg::Mesh& plg::Mesh::operator=(const Mesh& other) {
	if (this != &other) {
		m_Vertices = other.m_Vertices;
		m_Edges = other.m_Edges;
		m_Faces = other.m_Faces;
	}
	return *this;
}

plg::Mesh& plg::Mesh::operator=(Mesh&& other) noexcept {
	if (this != &other) {
		m_Vertices = std::move(other.m_Vertices);
		m_Edges = std::move(other.m_Edges);
		m_Faces = std::move(other.m_Faces);
	}
	return *this;
}

void plg::Mesh::AddVertex(plg::Vertex object) {
	m_Vertices.Append(object);
}

void plg::Mesh::AddEdge(plg::Edge object) {

}

void plg::Mesh::AddFace(plg::Face object) {

}

void plg::Mesh::RotateEdge(Edge edge, float angle) {
	Vec2 normal(std::cosf(angle), std::sinf(angle));
	m_Vertices[edge.m_End].RotateByVecIP(normal, m_Vertices[edge.m_Start]);
}

void plg::Mesh::RotateEdge(Edge edge, Vec2 normal) {
	m_Vertices[edge.m_End].RotateByVecIP(normal, m_Vertices[edge.m_Start]);
}

void plg::Mesh::RotateByCenterEdge(Edge edge, float angle) {
	Vec2 center((m_Vertices[edge.m_Start] + m_Vertices[edge.m_End]) / 2);
	Vec2 normal(std::cosf(angle), std::sinf(angle));
	m_Vertices[edge.m_Start].RotateByVecIP(normal, center);
	m_Vertices[edge.m_End].RotateByVecIP(normal, center);
}

void plg::Mesh::RotateByCenterEdge(Edge edge, Vec2 normal) {
	Vec2 center((m_Vertices[edge.m_Start] + m_Vertices[edge.m_End]) / 2);
	m_Vertices[edge.m_Start].RotateByVecIP(normal, center);
	m_Vertices[edge.m_End].RotateByVecIP(normal, center);
}

void plg::Mesh::RotateByCentroidEdge(Edge edge, float angle, Vec2 centroid) {
	Vec2 normal(std::cosf(angle), std::sinf(angle));
	m_Vertices[edge.m_Start].RotateByVecIP(normal, centroid);
	m_Vertices[edge.m_End].RotateByVecIP(normal, centroid);
}

void plg::Mesh::RotateByCentroidEdge(Edge edge, Vec2 normal, Vec2 centroid) {
	m_Vertices[edge.m_Start].RotateByVecIP(normal, centroid);
	m_Vertices[edge.m_End].RotateByVecIP(normal, centroid);
}

void plg::Mesh::MoveEdge(Edge edge, Vec2 offset) {
	m_Vertices[edge.m_Start].AddVec(offset);
	m_Vertices[edge.m_End].AddVec(offset);
}

void plg::Mesh::MoveFace(Face face, Vec2 offset) {
	m_Vertices[face.m_Vert1].AddVec(offset);
	m_Vertices[face.m_Vert2].AddVec(offset);
	m_Vertices[face.m_Vert3].AddVec(offset);
}

plg::Vec2 plg::Mesh::GetEdgeCenter(Edge edge) {
	Vec2 center = m_Vertices[edge.m_Start] + m_Vertices[edge.m_End];
	return center / 2;
}

plg::Vec2 plg::Mesh::GetFaceCenter(Face face) {
	Vec2 center = m_Vertices[face.m_Vert1] + m_Vertices[face.m_Vert2] + m_Vertices[face.m_Vert3];
	return center / 3;
}

void plg::Mesh::Render(SDL_Renderer* renderer, Vec2 offset) {
	SDL_SetRenderDrawColor(renderer, 76, 156, 216, SDL_ALPHA_OPAQUE);
	for (auto it_edge = m_Edges.Begin(); it_edge < it_edge.end_ptr; it_edge++) {
		SDL_RenderDrawLine(renderer, (int)it_edge->GetStart(&m_Vertices).x, (int)it_edge->GetStart(&m_Vertices).y, (int)it_edge->GetEnd(&m_Vertices).x, (int)it_edge->GetEnd(&m_Vertices).y);
	}
	if (sceneMeshData.GetMode() == MeshMode::PLG_EDGE) {
		SDL_SetRenderDrawColor(renderer, 216, 116, 56, SDL_ALPHA_OPAQUE);
		for (auto edge_it = sceneMeshData.GetEdgeIter(); edge_it < edge_it.end_ptr; edge_it++) {
			Edge edge = m_Edges[*edge_it];
			SDL_RenderDrawLine(renderer, (int)m_Vertices[edge.m_Start].x, (int)m_Vertices[edge.m_Start].y, (int)m_Vertices[edge.m_End].x, (int)m_Vertices[edge.m_End].y);
		}
	}
	else if (sceneMeshData.GetMode() == MeshMode::PLG_VERTEX) {
		for (auto it_vertex = m_Vertices.Begin(); it_vertex < it_vertex.end_ptr; it_vertex++) {
			drawCircleFilled(renderer, it_vertex->x, it_vertex->y, 2, { 216, 216, 216, SDL_ALPHA_OPAQUE });
		}
		for (auto vertex_it = sceneMeshData.GetVertexIter(); vertex_it < vertex_it.end_ptr; vertex_it++) {
			Vec2 vertex = m_Vertices[*vertex_it];
			drawCircleFilled(renderer, vertex.x, vertex.y, 2, { 216, 116, 56, SDL_ALPHA_OPAQUE });
		}
	}
	else if (sceneMeshData.GetMode() == MeshMode::PLG_FACE) {
		for (auto it_face = m_Faces.Begin(); it_face < it_face.end_ptr; it_face++) {
			drawPolygon(renderer, { m_Vertices[it_face->m_Vert1], m_Vertices[it_face->m_Vert2], m_Vertices[it_face->m_Vert3] }, { 20, 20, 20, SDL_ALPHA_OPAQUE });
			drawRawPolygon(renderer, { m_Vertices[it_face->m_Vert1], m_Vertices[it_face->m_Vert2], m_Vertices[it_face->m_Vert3] }, { 216, 216, 216, SDL_ALPHA_OPAQUE });
			Vec2 center = GetFaceCenter(*it_face);
			drawCircleFilled(renderer, center.x, center.y, 2, { 216, 216, 216, SDL_ALPHA_OPAQUE });
		}
		for (auto face_it = sceneMeshData.GetFaceIter(); face_it < face_it.end_ptr; face_it++) {
			Face face = m_Faces[*face_it];
			drawPolygon(renderer, { m_Vertices[face.m_Vert1], m_Vertices[face.m_Vert2], m_Vertices[face.m_Vert3] }, { 36, 30, 20, SDL_ALPHA_OPAQUE });
			drawRawPolygon(renderer, { m_Vertices[face.m_Vert1], m_Vertices[face.m_Vert2], m_Vertices[face.m_Vert3] }, { 216, 116, 56, SDL_ALPHA_OPAQUE });
			Vec2 center = GetFaceCenter(face);
			drawCircleFilled(renderer, center.x, center.y, 2, { 216, 116, 56, SDL_ALPHA_OPAQUE });
		}
	}
}

bool plg::SceneMeshData::SetMesh(container::List<Mesh>* meshList, Vec2 mousePos) {
	return true;
}

bool plg::SceneMeshData::SetVertex(Mesh* mesh, Vec2 mousePos) {
	for (auto iter = mesh->GetVertexIter(); iter < iter.end_ptr; iter++)
		if (s_CollideVertex(*iter, mousePos)) {
			m_Cleared = false;
			m_SelectedVertices.Append((int)(iter.operator->() - iter.GetBegin()));
			Log("Selected Vertex.", true);
			return true;
		}
	return false;
}

bool plg::SceneMeshData::SetEdge(Mesh* mesh, Vec2 mousePos) {
	for (auto iter = mesh->GetEdgeIter(); iter < iter.end_ptr; iter++)
		if (s_CollideEdge(*iter, mesh->GetVertexList(), mousePos)) {
			m_Cleared = false;
			m_SelectedEdges.Append((int)(iter.operator->() - iter.GetBegin()));
			return true;
		}
	return false;
}

bool plg::SceneMeshData::SetFace(Mesh* mesh, Vec2 mousePos) {
	for (auto iter = mesh->GetFaceIter(); iter < iter.end_ptr; iter++)
		if (s_CollideFace(*iter, mesh->GetVertexList(), mousePos)) {
			m_Cleared = false;
			m_SelectedFaces.Append((int)(iter.operator->() - iter.GetBegin()));
			return true;
		}
	return false;
}

void plg::SceneMeshData::SetMode(uint8_t mode) {
	switch (mode) {
	case 0:
		m_Mode = MeshMode::PLG_VERTEX;
		break;
	case 1:
		m_Mode = MeshMode::PLG_EDGE;
		break;
	case 2:
		m_Mode = MeshMode::PLG_FACE;
		break;
	}
}

void plg::SceneMeshData::Clear() {
	m_SelectedVertices.Clear();
	m_SelectedEdges.Clear();
	m_SelectedFaces.Clear();
	m_Cleared = true;
}

plg::SceneMeshData plg::sceneMeshData = plg::SceneMeshData();