#ifndef CHARACTERCONTROLLER_H
#define CHARACTERCONTROLLER_H

#include <map>
#include "AnimationController.h"
#include "irrlicht.h"
#include "btBulletDynamicsCommon.h"

enum ActionType
{
	AT_NONE = 1,
	AT_MOVELEFT = 2,
	AT_MOVERIGHT = 4,
	AT_MOVEUP = 8,
	AT_MOVEDOWN = 16,
	AT_ATTACK1 = 32,
	AT_ATTACK2 = 64,
	AT_ATTACK3 = 128,
	AT_ATTACK4 = 256,
	AT_EQUIP1 = 512,
	AT_EQUIP2 = 1024,
	AT_EQUIP3 = 2048,
	//the RELEASE actions contain the bits for NONE and the corresponding action
	AT_RELEASELEFT = 3,
	AT_RELEASERIGHT = 5,
	AT_RELEASEUP = 9,
	AT_RELEASEDOWN = 17,
	AT_RELEASEATTACK1 = 33,
	AT_RELEASEATTACK2 = 65,
	AT_RELEASEATTACK3 = 129,
	AT_RELEASEATTACK4 = 257
};

typedef std::map<irr::EKEY_CODE, ActionType> KeyActionMap;

enum VelocityState
{
	VS_NONE = 0,
	VS_ACCELERATING,
	VS_STABLE,
	VS_DECELERATING,
	VS_MAX
};

enum LookDirection
{
	LD_LEFT = 0,
	LD_RIGHT	//probably the default
};

//basic things that every character has
struct CharacterData
{
	double health;
	double stamina;
	
	btVector3 maxSpeed;
	btVector3 acceleration;

	VelocityState velocityState;
	LookDirection lookDirection;
};

const int MaxActionHistory = 8;

//ties a rigid body to a set of performable actions
//whether it is a player controlled, AI controlled or (potentially) network controlled, each must provide the following functions...
//in a proper project I'd separate these into their own files for ease
class CharacterController
{
	public:
		virtual void Init(btRigidBody *bodyPtr, const CharacterData &charInfo, AnimationController *animCtrl) = 0;

		virtual void Action(ActionType action, btDiscreteDynamicsWorld *world) = 0;	//actions happen on demand
		virtual void Update(irr::u32 delta) = 0;	//updates happen every frame - useful for things like damage effects, key combos, tracking velocity, etc.

		virtual bool CheckGrounded(const btDiscreteDynamicsWorld *world) = 0;

		virtual void SetGrounded(bool onGround) = 0;
		virtual bool GetGrounded() = 0;

		virtual btRigidBody* GetRigidBody() = 0;
		virtual AnimationController* GetAnimationController() = 0;
};

class PlayerCharacterController : public CharacterController
{
	private:
		AnimationController *m_animCtrl;
		btRigidBody *m_body;
		CharacterData m_charInfo;
		ActionType prevActions[MaxActionHistory];
		btVector3 prevVelocity[MaxActionHistory];
		bool	m_grounded;
		btQuaternion m_initialOrientation;
		btQuaternion m_reverseOrientation;	//facing the other way, prevents recalculating this all the time
		btVector3 m_initialEulers;
		btVector3 m_reverseEulers;

	public:
		PlayerCharacterController();
		PlayerCharacterController(btRigidBody *bodyPtr, AnimationController *animCtrl);
		PlayerCharacterController(btRigidBody *bodyPtr, const CharacterData &charInfo, AnimationController *animCtrl, KeyActionMap keyActionMap);
		~PlayerCharacterController(){}

		KeyActionMap m_keyActionMap;

		void Init(btRigidBody *bodyPtr, const CharacterData &charInfo, AnimationController *animCtrl);

		void Action(ActionType action, btDiscreteDynamicsWorld *world);
		void Update(irr::u32 delta);

		bool CheckGrounded(const btDiscreteDynamicsWorld *world);

		void SetGrounded(bool onGround) { m_grounded = onGround; }
		bool GetGrounded() { return m_grounded; }

		CharacterData GetCharacterInfo() { return m_charInfo; }

		btRigidBody* GetRigidBody() { return m_body; }
		AnimationController* GetAnimationController() { return m_animCtrl; }
};

#endif