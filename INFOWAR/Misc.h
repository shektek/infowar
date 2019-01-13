#ifndef MISC_H
#define MISC_H

#include "irrlicht.h"
#include "btBulletCollisionCommon.h"

void QuaternionToEuler(const btQuaternion &quat, btVector3 &euler);

#endif