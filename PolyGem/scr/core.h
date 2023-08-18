#pragma once
#include <iostream>
#include "benchmark.h"
#include "container.h"
#include <memory>

namespace plg {
	class Vec2 {
	public:
		float x, y;


		Vec2(float xv = 0.0f, float yv = 0.0f) : x(xv), y(yv) {}

		~Vec2() {}

		void Invert() {
			x = -x;
			y = -y;
		}
		float Magnitude() const {
			if (x == 0.0f && y == 0.0f) { return 0; }
			return (sqrtf(SquareMagnitude()));
		}
		float SquareMagnitude() const { return (x * x + y * y); }
		void Normalize();
		float NormalReturnMag();
		void SetZero() {
			x = 0.0f;
			y = 0.0f;
		}
		Vec2 Rotate(float angle);
		Vec2 Rotate(float angle, const Vec2& centroid);
		void RotateIP(float angle);
		void RotateIP(float angle, const Vec2& centroid);
		Vec2 RotateByVec(const Vec2& normal) { return Vec2(x * normal.x - y * normal.y, x * normal.y + y * normal.x); }
		Vec2 RotateByVec(const Vec2& normal, const Vec2& centroid);
		void RotateByVecIP(const Vec2& normal);
		void RotateByVecIP(const Vec2& normal, const Vec2& centroid);
		void operator *= (const float value) {
			x *= value;
			y *= value;
		}
		Vec2 operator* (const float value) {
			return Vec2(x * value, y * value);
		}
		void operator /= (const float value) {
			x /= value;
			y /= value;
		}
		Vec2 operator/ (const float value) {
			return Vec2(x / value, y / value);
		}
		void operator += (const Vec2& vec) {
			x += vec.x;
			y += vec.y;
		}
		Vec2 operator+ (const Vec2& vec) {
			return Vec2(x + vec.x, y + vec.y);
		}
		void operator -= (const Vec2& vec) {
			x -= vec.x;
			y -= vec.y;
		}
		Vec2 operator- (const Vec2& vec) {
			return Vec2(x - vec.x, y - vec.y);
		}
		Vec2 ComponentProduct(const Vec2& vec){ return Vec2(x * vec.x, y * vec.y); }
		void ComponentProductUpdate(const Vec2& vec) {
			x *= vec.x;
			y *= vec.y;
		}
		float ScalarProduct(const Vec2& vec){ return x * vec.x + y * vec.y; }
		void ConjugateWith(Vec2& vec);
		float operator* (const Vec2& vec) { return x * vec.x + y * vec.y; }
		void AddVec(const Vec2& vec) {
			x += vec.x;
			y += vec.y;
		}void AddScaledVec(const Vec2& vec, float scale) {
			x += vec.x * scale;
			y += vec.y * scale;
		}
		std::ostream& Print(std::ostream& stream) {
			return stream << "Vec2< " << x << ", " << y << " >";
		}
	};

	class Vertex : public Vec2 {
	public:
		Vertex(float xX, float yY) : Vec2(xX, yY) { }
		
		~Vertex() { }
	};

	class Edge {
	public:
		Edge(size_t start, size_t end, container::List<Vertex>* vertexMesh) : m_Start(start), m_End(end), m_VertexMesh(vertexMesh) { }

		~Edge() { }

		void Rotate(float angle);
		void Rotate(Vec2 normal);
		void RotateByCenter(float angle);
		void RotateByCenter(Vec2 normal);
		void RotateByCentroid(float angle, Vec2 centroid);
		void RotateByCentroid(Vec2 normal, Vec2 centroid);
		void Move(Vec2 offset);
		
		Vertex GetStart() { return m_VertexMesh->operator[](m_Start); }
		Vertex GetEnd() { return m_VertexMesh->operator[](m_End); }

	private:
		size_t m_Start;
		size_t m_End;
		container::List<Vertex>* m_VertexMesh;
	};
	
	class Face {
	public:

		Face(std::initializer_list<size_t> indices, container::List<Vertex>* vertices) {
			m_VertexIndices = container::List<size_t>(indices);
			m_Vertices = vertices;
		}



		~Face() {
			m_Vertices = nullptr;
		}

		void Move(Vec2 offset);
		Vec2 GetCenter();
		static bool CheckValidLines(std::initializer_list<Edge*> lines);
		static Face MakeFaceFromLines(std::initializer_list<Edge*> lines, const container::ListIterator<container::List<Vertex>> vertex_iter);

	private:
		container::List<size_t> m_VertexIndices;
		container::List<Vertex>* m_Vertices;
	};
}