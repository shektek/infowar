#include "Misc.h"

void QuaternionToEuler(const btQuaternion &quat, btVector3 &euler)
{
	btScalar w = quat.getW(), x = quat.getX(), y = quat.getY(), z = quat.getZ();
	
	btScalar sinr = 2.0 * (w * x + y * z);
	btScalar cosr = 1.0 - 2.0 * (x * x + y * y);
	euler.setX(atan2(sinr, cosr));

	btScalar sinp = 2.0 * (w * y - z * x);
	if (fabs(sinp) >= 1)
		euler.setY(copysign(SIMD_HALF_PI, sinp));
	else
		euler.setY(asin(sinp));

	btScalar siny = 2.0 * (w * z + x * y);
	btScalar cosy = 1.0 - 2.0 * (y * y + z * z);
	euler.setZ(atan2(siny, cosy));

	euler *= irr::core::RADTODEG;

	/*btScalar coords[4] = { quat.getW(), quat.getX(), quat.getY(), quat.getZ() };
	double squares[4] = { coords[0] * coords[0], coords[1] * coords[1], coords[2] * coords[2], coords[3] * coords[3] };

	euler.setX(atan2(2.0 * (coords[2] * coords[3] + coords[1] * coords[0]), -squares[1] - squares[2] + squares[3] + squares[0]));
	euler.setY(asin(-2.0 * (coords[1] * coords[3] - coords[2] * coords[0])));
	euler.setZ(atan2(2.0 * (coords[1] * coords[2] + coords[3] * coords[0]), squares[1] - squares[2] - squares[3] + squares[0]));
	euler *= irr::core::RADTODEG;*/
}