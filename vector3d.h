#ifndef _VECTOR3D_H_
#define _VECTOR3D_H_
#include <iostream>
#include <math.h>

#ifndef PI
const double PI         = 3.141592653589793238462643383279502884197169399375105820974944592307816;
#endif
const double PId        = PI;
const float  PIf        = static_cast<float>(PI);

#ifndef HALFPI
const double HALFPI     = PI / 2.0;
#endif
const double HALFPId    = HALFPI;
const float  HALFPIf    = static_cast<float>(HALFPI);

#ifndef TWOPI
const double TWOPI      = 2.0 * PI;
#endif
const double TWOPId     = TWOPI;
const float  TWOPIf     = static_cast<float>(TWOPI);

#ifndef INVPI
const double INVPI      = 1.0 / PI;
#endif
const double INVPId     = INVPI;
const float  INVPIf     = static_cast<float>(INVPI);

#ifndef PIOVER180
const double PIOVER180		= PI/180.0;
#endif
const double PIOVER180d		= PIOVER180;
const float PIOVER180f		= static_cast<float>(PIOVER180);


class Vec3D {
public:
	float x,y,z;

	Vec3D(float x0 = 0.0f, float y0 = 0.0f, float z0 = 0.0f) : x(x0), y(y0), z(z0) {}

	Vec3D(const Vec3D& v) : x(v.x), y(v.y), z(v.z) {}

	void reset() {
		x = y = z = 0.0f;
	}

	Vec3D& operator= (const Vec3D &v) {
        x = v.x;
		y = v.y;
		z = v.z;
		return *this;
	}

	/*
	float[3] operator= (const Vec3D &v) {
		float f[3] = {v.x, v.y, v.z};
		return f;
	}
	*/

	Vec3D operator+ (const Vec3D &v) const
	{
		Vec3D r(x+v.x,y+v.y,z+v.z);
        return r;
	}

	Vec3D operator- (const Vec3D &v) const
	{
		Vec3D r(x-v.x,y-v.y,z-v.z);
		return r;
	}

	float operator* (const Vec3D &v) const
	{
        return x*v.x + y*v.y + z*v.z;
	}

	Vec3D operator* (float d) const
	{
		Vec3D r(x*d,y*d,z*d);
        return r;
	}

	Vec3D operator/ (float d) const
	{
		Vec3D r(x/d,y/d,z/d);
        return r;
	}

	friend Vec3D operator* (float d, const Vec3D& v)
	{
		return v * d;
	}

	// Cross Product
	Vec3D operator% (const Vec3D &v) const
	{
        Vec3D r(y*v.z-z*v.y, z*v.x-x*v.z, x*v.y-y*v.x);
		return r;
	}

	Vec3D& operator+= (const Vec3D &v)
	{
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}

	Vec3D& operator-= (const Vec3D &v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	}

	Vec3D& operator*= (float d)
	{
		x *= d;
		y *= d;
		z *= d;
		return *this;
	}

	float lengthSquared() const
	{
		return x*x+y*y+z*z;
	}

	float length() const
	{
        return sqrtf(x*x+y*y+z*z);
	}

	Vec3D& normalize()
	{
		this->operator*= (1.0f/length());
		return *this;
	}

	Vec3D operator~ () const
	{
		Vec3D r(*this);
        r.normalize();
		return r;
	}

	friend std::istream& operator>>(std::istream& in, Vec3D& v)
	{
		in >> v.x >> v.y >> v.z;
		return in;
	}

	friend std::ostream& operator<<(std::ostream& out, Vec3D& v)
	{
		out << "(" << v.x << ", " << v.y << ", " << v.z << ")";
		return out;
	}

	operator float*()
	{
		return (float*)this;
	}

};

class Vec2D {
public:
	float x,y;
	
	Vec2D(float x0 = 0.0f, float y0 = 0.0f) : x(x0), y(y0) {}

	Vec2D(const Vec2D& v) : x(v.x), y(v.y) {}

	Vec2D& operator= (const Vec2D &v) {
        x = v.x;
		y = v.y;
		return *this;
	}

	Vec2D operator+ (const Vec2D &v) const
	{
		Vec2D r(x+v.x,y+v.y);
        return r;
	}

	Vec2D operator- (const Vec2D &v) const
	{
		Vec2D r(x-v.x,y-v.y);
		return r;
	}

	float operator* (const Vec2D &v) const
	{
        return x*v.x + y*v.y;
	}

	Vec2D operator* (float d) const
	{
		Vec2D r(x*d,y*d);
        return r;
	}

	friend Vec2D operator* (float d, const Vec2D& v)
	{
		return v * d;
	}

	Vec2D& operator+= (const Vec2D &v)
	{
		x += v.x;
		y += v.y;
		return *this;
	}

	Vec2D& operator-= (const Vec2D &v)
	{
		x -= v.x;
		y -= v.y;
		return *this;
	}

	Vec2D& operator*= (float d)
	{
		x *= d;
		y *= d;
		return *this;
	}

	float lengthSquared() const
	{
		return x*x+y*y;
	}

	float length() const
	{
        return sqrtf(x*x+y*y);
	}

	Vec2D& normalize()
	{
		this->operator*= (1.0f/length());
		return *this;
	}

	Vec2D operator~ () const
	{
		Vec2D r(*this);
        r.normalize();
		return r;
	}


	friend std::istream& operator>>(std::istream& in, Vec2D& v)
	{
        in >> v.x >> v.y;
		return in;
	}

	friend std::ostream& operator<<(std::ostream& out, Vec2D& v)
	{
		out << "(" << v.x << ", " << v.y << ")";
		return out;
	}

	operator float*()
	{
		return (float*)this;
	}

};

class Vec4D {
public:
	float x,y,z,w;

	Vec4D(float x0=0.0f, float y0=0.0f, float z0=0.0f, float w0=0.0f) : x(x0), y(y0), z(z0), w(w0) {}

	Vec4D(const Vec4D& v) : x(v.x), y(v.y), z(v.z), w(v.w) {}

	Vec4D(const Vec3D& v, const float w0) : x(v.x), y(v.y), z(v.z), w(w0) {}

	Vec4D& operator= (const Vec4D &v) {
		x = v.x;
		y = v.y;
		z = v.z;
		w = v.w;
		return *this;
	}

	const Vec4D operator+ (const Vec4D &v) const
	{
		Vec4D r(x+v.x,y+v.y,z+v.z,w+v.w);
		return r;
	}

	const Vec4D operator- (const Vec4D &v) const
	{
		Vec4D r(x-v.x,y-v.y,z-v.z,w-v.w);
		return r;
	}

	const Vec4D operator* (float d) const
	{
		Vec4D r(x*d,y*d,z*d,w*d);
		return r;
	}

	friend Vec4D operator* (float d, const Vec4D& v)
	{
		return v * d;
	}

	Vec4D& operator+= (const Vec4D &v)
	{
		x += v.x;
		y += v.y;
		z += v.z;
		w += v.w;
		return *this;
	}

	Vec4D& operator-= (const Vec4D &v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		w -= v.w;
		return *this;
	}

	Vec4D& operator*= (float d)
	{
		x *= d;
		y *= d;
		z *= d;
		w *= d;
		return *this;
	}

	float operator* (const Vec4D &v) const
	{
		return x*v.x + y*v.y + z*v.z + w*v.w;
	}

	friend std::ostream& operator<<(std::ostream& out, Vec4D& v)
	{
		out << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
		return out;
	}

	float lengthSquared() const
	{
		return x*x+y*y+z*z+w*w;
	}

	float length() const
	{
		return sqrtf(x*x+y*y+z*z+w*w);
	}

	Vec4D& normalize()
	{
		this->operator*= (1.0f/length());
		return *this;
	}

	operator float*()
	{
		return (float*)this;
	}

	Vec3D xyz() const
	{
		return Vec3D(x,y,z);
	}
};

class Quaternion: public Vec4D {
public:
	Quaternion(float x0=0.0f, float y0=0.0f, float z0=0.0f, float w0=1.0f): Vec4D(x0,y0,z0,w0) {}

	Quaternion(const Vec4D& v) : Vec4D(v) {}

	Quaternion(const Vec3D& v, const float w0) : Vec4D(v, w0) {}

	static const Quaternion slerp(const float r, const Quaternion &v1, const Quaternion &v2)
	{
		// SLERP
		float dot = v1*v2;

		if (fabs(dot) > 0.9995f) {
			// fall back to LERP
			return Quaternion::lerp(r, v1, v2);
		}

		float a = acosf(dot) * r;
		Quaternion q = (v2 - v1 * dot);
		q.normalize();

		return v1 * cosf(a) + q * sinf(a);
	}

	static const Quaternion lerp(const float r, const Quaternion &v1, const Quaternion &v2)
	{
		return v1*(1.0f-r) + v2*r;
	}

	Vec3D GetHPB()
	{
		Vec3D hpb;
		hpb.x = atan2(2 * (x*z + y*w), 1 - 2 * (x*x + y*y));
		float sp = 2*(x*w - y*z);
		if(sp < -1) sp = -1;
		else if(sp > 1) sp = 1;
		hpb.y = asin(sp);
		hpb.z = atan2(2 * (x*y + z*w), 1 - 2 * (x*x + z*z));

		return hpb;
	}

};


inline void rotate(float x0, float y0, float *x, float *y, float angle)
{
	float xa = *x - x0, ya = *y - y0;
	*x = xa*cosf(angle) - ya*sinf(angle) + x0;
	*y = xa*sinf(angle) + ya*cosf(angle) + y0;
}

//我暫時只是為了坐標的一致 這裡以後要改為適應整個世界坐標的坐標方式
Vec3D fixCoordSystem(Vec3D v);

Vec3D fixCoordSystem2(Vec3D v);

Quaternion fixCoordSystemQuat(Quaternion v);

#endif

