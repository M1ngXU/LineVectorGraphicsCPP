#include <string>

class Vector3 {
public:
	double x = 0;
	double y = 0;
	double z = 0;
	Vector3(double x = 0.0, double y = 0.0, double z = 0.0) {
		this->x = x;
		this->y = y;
		this->z = z;
	}
	Vector3 operator+(const Vector3& rhs) {
		return Vector3(this->x + rhs.x, this->y + rhs.y, this->z + rhs.z);
	}
	void operator+=(const Vector3& rhs) {
		this->x += rhs.x;
		this->y += rhs.y;
		this->z += rhs.z;
	}
	void operator-=(const Vector3& rhs) {
		this->x -= rhs.x;
		this->y -= rhs.y;
		this->z -= rhs.z;
	}
	Vector3 operator-(const Vector3& rhs) {
		return Vector3(this->x - rhs.x, this->y - rhs.y, this->z - rhs.z);
	}
	Vector3 operator-() {
		return Vector3(-this->x, -this->y, -this->z);
	}
	Vector3 Cross(const Vector3& rhs) {
		return Vector3(
			this->y * rhs.z - this->z * rhs.y,
			this->z * rhs.x - this->x * rhs.z,
			this->x * rhs.y - this->y * rhs.x
		);
	}
	Vector3 operator*(const double& rhs) {
		return Vector3(this->x * rhs, this->y * rhs, this->z * rhs);
	}
	double operator*(const Vector3& rhs) {
		return this->x * rhs.x + this->y * rhs.y + this->z * rhs.z;
	}
	double operator[](const int i) {
		if (i == 0) return this->x;
		if (i == 1) return this->y;
		if (i == 2) return this->z;
		return 0;
	}
	std::string ToString() {
		return std::to_string(this->x)
			+ "|" + std::to_string(this->y)
			+ "|" + std::to_string(this->z);
	}
};