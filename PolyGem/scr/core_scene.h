#pragma once
#include "core.h"
#include "SDL.h"

namespace plg {
	using Vertex = Vec2;

	class Edge {
	public:
		Edge() { }
		Edge(size_t start, size_t end) : m_Start(start), m_End(end) { }
		Edge(const Edge& other) : m_Start(other.m_Start), m_End(other.m_End) { }
		Edge(Edge&& other) noexcept : m_Start(other.m_Start), m_End(other.m_End) { }
		Edge& operator=(const Edge& other);
		Edge& operator=(Edge&& other) noexcept;
		~Edge() { }

		Vertex& GetStart(container::List<Vertex>* vertexMesh) { return vertexMesh->operator[](m_Start); }
		Vertex& GetEnd(container::List<Vertex>* vertexMesh) { return vertexMesh->operator[](m_End); }
		bool operator==(const Edge& other) {
			return (m_Start == other.m_Start && m_End == other.m_End) || (m_Start == other.m_End && m_End == other.m_Start);
		}
		bool operator!=(const Edge& other) {
			return !(m_Start == other.m_Start && m_End == other.m_End) || (m_Start == other.m_End && m_End == other.m_Start);
		}
		
	public:
		int m_Start;
		int m_End;
	};

	class Face {
	public:
		Face() { }
		Face(int vert1, int vert2, int vert3) {
			m_Vert1 = vert1;
			m_Vert2 = vert2;
			m_Vert3 = vert3;
		}
		~Face() { }

	public:
		int m_Vert1 = -1;
		int m_Vert2 = -1;
		int m_Vert3 = -1;
	};
	
	enum class MeshMode {
		PLG_VERTEX, PLG_EDGE, PLG_FACE
	};

	class Mesh {
	public:
		Mesh() { }
		Mesh(std::initializer_list<Vertex> vertices);
		Mesh(const Mesh& other);
		Mesh(Mesh&& other) noexcept;
		Mesh& operator=(const Mesh& other);
		Mesh& operator=(Mesh&& other) noexcept;
		~Mesh() { }

		container::List<Vertex>* GetVertexList() { return &m_Vertices; }
		container::ListIterator<container::List<Vertex>> GetVertexIter() { return m_Vertices.Begin(); }
		container::List<Edge>* GetEdgeList() { return &m_Edges; }
		container::ListIterator<container::List<Edge>> GetEdgeIter() { return m_Edges.Begin(); }
		container::List<Face>* GetFaceList() { return &m_Faces; }
		container::ListIterator<container::List<Face>> GetFaceIter() { return m_Faces.Begin(); }
		void RotateEdge(Edge edge, float angle);
		void RotateEdge(Edge edge, Vec2 normal);
		void RotateByCenterEdge(Edge edge, float angle);
		void RotateByCenterEdge(Edge edge, Vec2 normal);
		void RotateByCentroidEdge(Edge edge, float angle, Vec2 centroid);
		void RotateByCentroidEdge(Edge edge, Vec2 normal, Vec2 centroid);
		void MoveEdge(Edge edge, Vec2 offset);
		void MoveFace(Face face, Vec2 offset);
		Vec2 GetEdgeCenter(Edge edge);
		Vec2 GetFaceCenter(Face face);
		int32_t AddVertex(Vertex object);
		int32_t AddEdge(Edge object);
		int32_t AddFace(Face object);
		void Render(SDL_Renderer* renderer, Vec2 offset);
		
	private:
		container::List<Vertex> m_Vertices;
		container::List<Edge> m_Edges;
		container::List<Face> m_Faces;
	};

	class SceneMeshData {
	public:
		SceneMeshData() { }

		static const int32_t NULL_MESH = -1;
		bool SetMesh(container::List<Mesh>* meshList, Vec2 mousePos);
		bool SetVertex(Mesh* mesh, Vec2 mousePos);
		bool SetEdge(Mesh* mesh, Vec2 mousePos);
		bool SetFace(Mesh* mesh, Vec2 mousePos);
		void SetMode(uint8_t mode);
		void Clear();
		MeshMode GetMode() { return m_Mode; }
		int GetMeshID() { return m_SelectedMeshID; }
		bool IsCleared() { return m_Cleared; }
		size_t GetVertexCount() { return m_SelectedVertices.GetSize(); }
		size_t GetEdgeCount() { return m_SelectedEdges.GetSize(); }
		size_t GetFaceCount() { return m_SelectedFaces.GetSize(); }

		container::ListIterator<container::List<int32_t>> GetVertexIter() { return m_SelectedVertices.Begin(); }
		container::ListIterator<container::List<int32_t>> GetEdgeIter() { return m_SelectedEdges.Begin(); }
		container::ListIterator<container::List<int32_t>> GetFaceIter() { return m_SelectedFaces.Begin(); }

	private:
		SceneMeshData(const SceneMeshData&) { }
		SceneMeshData& operator=(const SceneMeshData&) { }

		MeshMode m_Mode = MeshMode::PLG_VERTEX;
		int32_t m_SelectedMeshID = 0;
		bool m_Cleared = false;
		container::List<int32_t> m_SelectedVertices = container::List<int32_t>(4);
		container::List<int32_t> m_SelectedEdges = container::List<int32_t>(4);
		container::List<int32_t> m_SelectedFaces = container::List<int32_t>(4);
	};

	extern SceneMeshData sceneMeshData;
}