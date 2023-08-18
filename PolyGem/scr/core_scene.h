#pragma once
#include "core.h"
#include "SDL.h"

namespace plg {
	enum class MeshMode {
		PLG_VERTEX, PLG_EDGE, PLG_FACE
	};

	class Mesh {
	public:
		Mesh();
		Mesh(std::initializer_list<Vertex> vertices);
		Mesh(const Mesh& other);
		Mesh(Mesh&& other);
		Mesh& operator=(const Mesh& other);
		Mesh& operator=(Mesh&& other);

		~Mesh() { }

		void SetMode(MeshMode mode) { m_Mode = mode; }
		void AddVertex(Vertex object);
		void AddEdge(Vertex object);
		void AddFace(Face object);
		void Render(SDL_Renderer* renderer, Vec2 offset);
		
	private:
		MeshMode m_Mode;
		container::List<Vertex> m_Vertices;
		container::List<Edge> m_Edges;
		container::List<Face> m_Faces;
	};
}