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
		float GetDistanceTo(const Vec2 other) { return sqrtf((other.x - x) * (other.x - x) + (other.y - y) *(other.y - y)); }
		float GetDistancetoSquared(const Vec2 other) { return (other.x - x) * (other.x - x) + (other.y - y) * (other.y - y); }
		void Normalize();
		float NormalReturnMag();
		void Reset() {
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
		Vec2 ComponentProduct(const Vec2& vec) { return Vec2(x * vec.x, y * vec.y); }
		void ComponentProductUpdate(const Vec2& vec) {
			x *= vec.x;
			y *= vec.y;
		}
		float ScalarProduct(const Vec2& vec) { return x * vec.x + y * vec.y; }
		float CrossProduct(const Vec2& vec) { return x * vec.y - y * vec.x; }
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
}