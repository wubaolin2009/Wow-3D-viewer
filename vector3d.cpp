#include "vector3d.h"

Vec3D fixCoordSystem(Vec3D v)
{
	//return v;
	return Vec3D(v.x, v.z,-v.y);
}
Vec3D fixCoordSystem2(Vec3D v)
{
	return Vec3D(v.x, v.z, v.y);
}

Quaternion fixCoordSystemQuat(Quaternion v)
{
	return Quaternion(-v.x, -v.z, v.y, v.w);
}
