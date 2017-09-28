#pragma once

typedef class vector {
public:
	union {
		struct {
			float x,y,z;
		};
		float a1d[3];
	};

	vector() {x = y = z = 0.0f;}
	vector(const float x, const float y, const float z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}

	vector operator +(const vector &rhs)
	{
		return vector(x + rhs.x, y + rhs.y, z + rhs.z);
	}

	vector operator -(const vector &rhs)
	{
		return vector(x - rhs.x, y - rhs.y, z - rhs.z);
	}

	vector operator *(const float n)
	{
		return vector(	x * n, y * n, z * n);
	}

	vector operator /(const float n)
	{
		return vector(	x / n, y / n, z / n);
	}

	void operator +=(const vector &rhs)
	{
		x += rhs.x;
		y += rhs.y;
		z += rhs.z;
	}

	void operator -=(const vector &rhs)
	{
		x -= rhs.x;
		y -= rhs.y;
		z -= rhs.z;
	}

	void operator *=(const float n)
	{
		x *= n;
		y *= n;
		z *= n;
	}

	void operator /=(const float n)
	{
		x /= n;
		y /= n;
		z /= n;
	}

	const vector &operator =(const vector &rhs)
	{
		if(this == &rhs) 
		{
			// self assignment
			return *this;
		}

		x = rhs.x;
		y = rhs.y;
		z = rhs.z;

		return *this;
	}

	float LengthSquared();
	float Length();

	static float DotProduct(vector &v1, vector v2);

#if defined(FS2_UE)
	FVector Get()
	{
		return FVector(x, y, z);
	}
#endif

} vector;