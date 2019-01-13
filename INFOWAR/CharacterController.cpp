#include "CharacterController.h"
#include "Misc.h"

PlayerCharacterController::PlayerCharacterController()
{
	m_body = nullptr;
	memset((void*)&m_charInfo, 0, sizeof(m_charInfo));
	m_keyActionMap = KeyActionMap{
		{ irr::EKEY_CODE::KEY_LEFT, AT_MOVELEFT },
		{ irr::EKEY_CODE::KEY_RIGHT, AT_MOVERIGHT },
		{ irr::EKEY_CODE::KEY_SPACE, AT_MOVEUP },
		{ irr::EKEY_CODE::KEY_SHIFT, AT_ATTACK1 }
	};
}

PlayerCharacterController::PlayerCharacterController(btRigidBody *body, AnimationController *animCtrl)
{
	m_body = body;
	m_animCtrl = animCtrl;
	m_initialOrientation = m_body->getOrientation();
	QuaternionToEuler(m_initialOrientation, m_initialEulers);
	m_reverseEulers = btVector3(m_initialEulers.x(), m_initialEulers.y() + btRadians(180), m_initialEulers.z());
	m_reverseOrientation = btQuaternion(btVector3(0, 1, 0), SIMD_PI) * m_initialOrientation;

	m_charInfo.acceleration = {8, 70, 8};
	m_charInfo.maxSpeed = { 10, 80, 10 };
	m_charInfo.health = 100.0;
	m_charInfo.stamina = 100.0;
	m_charInfo.lookDirection = LD_RIGHT;
	m_keyActionMap = KeyActionMap{
		{ irr::EKEY_CODE::KEY_LEFT, AT_MOVELEFT },
		{ irr::EKEY_CODE::KEY_RIGHT, AT_MOVERIGHT },
		{ irr::EKEY_CODE::KEY_SPACE, AT_MOVEUP },
		{ irr::EKEY_CODE::KEY_LSHIFT, AT_ATTACK1 }
	};
}

PlayerCharacterController::PlayerCharacterController(btRigidBody *body, const CharacterData &charInfo, AnimationController *animCtrl, KeyActionMap keyActionMap)
{
	m_body = body;
	m_charInfo = charInfo;
	m_animCtrl = animCtrl;
	m_keyActionMap = keyActionMap;
}

void PlayerCharacterController::Init(btRigidBody *bodyPtr, const CharacterData &charInfo, AnimationController *animCtrl)
{
	m_body = bodyPtr;
	m_charInfo = charInfo;
	m_animCtrl = animCtrl;
}

void PlayerCharacterController::Action(ActionType action, btDiscreteDynamicsWorld *world)
{
	for (int i = 1; i < MaxActionHistory; i++)
	{
		prevActions[i - 1] = prevActions[i];
	}
	prevActions[MaxActionHistory - 1] = action;

	if (m_body)
	{
		btVector3 vel = m_body->getLinearVelocity();

		switch (action)
		{
		case AT_MOVELEFT:
		m_body->setFriction(0.0);
		if (abs(vel.x()) < m_charInfo.maxSpeed.x() || m_charInfo.lookDirection == LD_RIGHT)
		{
			printf("AT_MOVELEFT\n");
			if (m_charInfo.lookDirection == LD_RIGHT)
				m_body->setWorldTransform(btTransform(m_reverseOrientation, m_body->getWorldTransform().getOrigin()));

			m_charInfo.lookDirection = LD_LEFT;
			m_charInfo.velocityState = VS_ACCELERATING;

			btVector3 leftvec = m_charInfo.acceleration;
			leftvec.setX(-(leftvec.x())); //* (frameDelta * 0.001)));
			leftvec.setY(0);
			leftvec.setZ(0);
			m_body->activate(true);
			m_body->applyCentralImpulse(leftvec);

			if (m_grounded) m_animCtrl->SetAnimation("run", 30, ANIM_LOOP);
		}
		else
		{
			printf("ignored at_moveleft\n");
			m_charInfo.velocityState = VS_STABLE;
		}
		break;

		case AT_MOVERIGHT:
		m_body->setFriction(0.0);
		if (abs(vel.x()) < m_charInfo.maxSpeed.x() || m_charInfo.lookDirection == LD_LEFT)
		{
			printf("AT_MOVERIGHT\n");
			if (m_charInfo.lookDirection == LD_LEFT)
				m_body->setWorldTransform(btTransform(m_initialOrientation, m_body->getWorldTransform().getOrigin()));

			m_charInfo.lookDirection = LD_RIGHT;
			m_charInfo.velocityState = VS_ACCELERATING;

			btVector3 rightvec = m_charInfo.acceleration;
			rightvec.setX((rightvec.x()));// * (frameDelta * 0.001)));
			rightvec.setY(0);
			rightvec.setZ(0);
			m_body->activate(true);
			m_body->applyCentralImpulse(rightvec);

			if (m_grounded) m_animCtrl->SetAnimation("run", 30, ANIM_LOOP);
		}
		else
		{
			printf("ignored at_moveright\n");
			m_charInfo.velocityState = VS_STABLE;
		}
		break;

		//check two conditions:
		//1) we haven't just pressed up
		//2) we don't already have vertical velocity
		case AT_MOVEUP:
		m_body->setFriction(0.0);
		if (abs(vel.y()) < 1.0 && m_grounded)
		{
			//printf("ACCELERATING\n");
			m_animCtrl->SetAnimation("jump", 30, ANIM_PAUSE); //???
			m_charInfo.velocityState = VS_ACCELERATING;
			btVector3 upVec = m_charInfo.acceleration;
			upVec.setX(0);
			upVec.setY(m_charInfo.acceleration.y());
			upVec.setZ(0);
			m_body->activate(true);
			m_body->applyCentralImpulse(upVec);
		}
		break;

		case AT_RELEASELEFT:
		m_animCtrl->SetAnimation("stand", 30, ANIM_LOOP);
		m_body->setFriction(1.0);
		m_charInfo.velocityState = VS_DECELERATING;
		printf("AT_RELEASELEFT\n");
		break;

		case AT_RELEASERIGHT:
		m_animCtrl->SetAnimation("stand", 30, ANIM_LOOP);
		m_body->setFriction(1.0);
		m_charInfo.velocityState = VS_DECELERATING;
		printf("AT_RELEASERIGHT\n");
		break;

		case AT_RELEASEUP:

		break;

		case AT_ATTACK1:
		{
			m_animCtrl->SetAnimation("attack", 30, ANIM_CHANGE, "stand");
			
			//ray test in front
			btVector3 start = m_body->getCenterOfMassPosition();
			btVector3 end;
			btVector3 attackImpulse;
			if (m_charInfo.lookDirection == LD_LEFT)
			{
				end = start - btVector3(m_body->getCollisionShape()->getLocalScaling().x() * 6.0, 0, 0);
				attackImpulse = btVector3(-100, 0, 0);
			}
			else
			{
				end = start + btVector3(m_body->getCollisionShape()->getLocalScaling().x() * 6.0, 0, 0);
				attackImpulse = btVector3(100, 0, 0);
			}
			btCollisionWorld::ClosestRayResultCallback RayCallback(start, end);

			
			world->rayTest(start, end, RayCallback);

			if (RayCallback.hasHit())
			{
				printf("HIT\n");
				btRigidBody *target = (btRigidBody*)RayCallback.m_collisionObject;
				target->applyCentralImpulse(attackImpulse);
			}
		}
		break;

		case AT_NONE:
		default:
		break;
		}
	}
}

void PlayerCharacterController::Update(irr::u32 delta)
{
	for (int i = 1; i < MaxActionHistory; i++)
	{
		prevVelocity[i - 1] = prevVelocity[i];
	}
	prevVelocity[MaxActionHistory - 1] = m_body->getLinearVelocity();

	btVector3 vel = prevVelocity[MaxActionHistory - 1];

	int iva[3] = { abs(prevVelocity[MaxActionHistory - 1].x()) * 10, abs(prevVelocity[MaxActionHistory - 1].y()) * 10, abs(prevVelocity[MaxActionHistory - 1].z()) * 10 };
	int ivb[3] = { abs(prevVelocity[MaxActionHistory - 2].x()) * 10, abs(prevVelocity[MaxActionHistory - 2].y()) * 10, abs(prevVelocity[MaxActionHistory - 2].z()) * 10 };
	int ivc[3] = { abs(prevVelocity[MaxActionHistory - 3].x()) * 10, abs(prevVelocity[MaxActionHistory - 3].y()) * 10, abs(prevVelocity[MaxActionHistory - 3].z()) * 10 };

	if ((iva[0] == ivb[0] && ivb[0] == ivc[0]) && (iva[1] == ivb[1] && ivb[1] == ivc[1]))
	{
		m_charInfo.velocityState = VS_STABLE;
	}

	if ((iva[0] == ivb[0] && ivb[0] == 0) && (iva[1] == ivb[1] && ivb[1] == 0) && m_animCtrl->GetCurrentAnimation() != "attack")
	{
		m_animCtrl->SetAnimation("stand", 30, ANIM_LOOP);	//this is fine...
	}

	if (m_body)
	{
		btVector3 cp = m_body->getCenterOfMassPosition();
		btVector3 cv = m_body->getLinearVelocity();
		btQuaternion co = m_body->getOrientation();

		//push back toward central z plane
		if (cp.z() != 0)
		{
			if (cp.z() > 0)
			{
				m_body->applyCentralImpulse(btVector3(0.0, 0.0, -(m_charInfo.acceleration.z() * (cp.z()))));
			}
			else if (cp.z() < 0)
			{
				m_body->applyCentralImpulse(btVector3(0.0, 0.0, m_charInfo.acceleration.z() * -(cp.z())));
			}
		}

		if (m_charInfo.velocityState == VS_DECELERATING && m_grounded)
		{
			m_animCtrl->SetAnimation("crwalk", 30, ANIM_LOOP);
			cv.setX(-(cv.x() / 1.5));
			cv.setY(0.0);
			cv.setZ(0.0);
			m_body->applyCentralImpulse(cv);
		}

		//reorient
		//TODO: don't make it apply torque every frame
		if (m_charInfo.lookDirection == LD_LEFT)
		{
			btQuaternion dQuat = m_reverseOrientation * co.inverse();
			btVector3 dEuler;
			QuaternionToEuler(dQuat, dEuler);
			m_body->applyTorqueImpulse(dEuler*1.5);
		}
		else
		{
			btQuaternion dQuat = m_initialOrientation * co.inverse();
			btVector3 dEuler;
			QuaternionToEuler(dQuat, dEuler);
			m_body->applyTorqueImpulse(dEuler*1.5);
		}
	}
}

bool PlayerCharacterController::CheckGrounded(const btDiscreteDynamicsWorld *world)
{
	bool ret = false;

	if (m_body)
	{
		btVector3 start = m_body->getCenterOfMassPosition();
		btVector3 end = start - btVector3(0, m_body->getCollisionShape()->getLocalScaling().y() * 3.0, 0);
		btCollisionWorld::ClosestRayResultCallback RayCallback(start, end);

		world->rayTest(start, end, RayCallback);

		if (RayCallback.hasHit())
		{
			ret = true;
		}
	}

	return ret;
}
