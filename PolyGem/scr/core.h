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
		void _Rotate(float angle);
		void _Rotate(float angle, const Vec2& centroid);
		Vec2 RotateByVec(const Vec2& normal) { return Vec2(x * normal.x - y * normal.y, x * normal.y + y * normal.x); }
		Vec2 RotateByVec(const Vec2& normal, const Vec2& centroid);
		void _RotateByVec(const Vec2& normal) {
			x = x * normal.x - y * normal.y;
			y = x * normal.y + y * normal.x;
		}
		void _RotateByVec(const Vec2& normal, const Vec2& centroid);
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
		Vertex(float xX, float yY) : Vec2(xX, yY) {}
		
		~Vertex() {}

		void IncRef() { m_RefCount++; }
		void DecRef() { m_RefCount--; }

	private:
		size_t m_RefCount = 1;
	};

	class Line {
	public:
		Line(Vertex* start, Vertex* end) : m_Start(start), m_End(end) {
			m_Start->IncRef();
			m_End->IncRef();
		}

		~Line() {
			m_Start->DecRef();
			m_End->DecRef();
		}

		void Rotate(float angle) { m_End->_Rotate(angle); }
		void Rotate(Vec2 normal) {m_End->_RotateByVec(normal); }
		void RotateByCenter(float angle);
		void RotateByCenter(Vec2 normal);
		void RotateByCentroid(float angle, Vec2 centroid);
		void RotateByCentroid(Vec2 normal, Vec2 centroid);
		void Move(Vec2 offset);
		
	private:
		Vertex* m_Start;
		Vertex* m_End;
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
		static bool CheckValidLines(std::initializer_list<Line*> lines);
		static Face MakeFaceFromLines(std::initializer_list<Line*> lines, const container::ListIterator<container::List<Vertex>> vertex_iter);

	private:
		container::List<size_t> m_VertexIndices;
		container::List<Vertex>* m_Vertices;
	};
}