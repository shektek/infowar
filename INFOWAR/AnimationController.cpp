#include "AnimationController.h"

MD2AnimationController::MD2AnimationController()
{
	m_modelNode = nullptr;
	m_endCallback = nullptr;
}

MD2AnimationController::MD2AnimationController(irr::scene::IAnimatedMeshSceneNode *modelNode)
{
	m_modelNode = modelNode;
	m_endCallback = new EndCallback();
}

MD2AnimationController::~MD2AnimationController()
{
	m_modelNode->setAnimationEndCallback(0);
	delete m_endCallback;
}

void MD2AnimationController::Init(irr::scene::IAnimatedMeshSceneNode *modelNode)
{
	m_modelNode = modelNode;
	m_endCallback = new EndCallback();
}

void MD2AnimationController::SetAnimation(std::string animName, int fps, EndBehaviour onEnd, std::string switchToThis)
{
	if (animName != m_curAnim)
	{
		m_curAnim = animName;
		m_modelNode->setMD2Animation(animName.c_str());
	}

	m_modelNode->setAnimationSpeed(fps);

	switch (onEnd)
	{
		case ANIM_PAUSE:
		m_modelNode->setAnimationEndCallback(0);
		m_modelNode->setLoopMode(false);
		break;

		case ANIM_CHANGE:
		m_modelNode->setLoopMode(false);
		if (switchToThis.length() == 0) switchToThis = "stand";
		m_endCallback->SetNextAnimation(switchToThis);
		m_modelNode->setAnimationEndCallback(m_endCallback);
		break;

		default:
		case ANIM_LOOP:
		m_modelNode->setAnimationEndCallback(0);
		m_modelNode->setLoopMode(true);
		break;
	}
}

void MD2AnimationController::SetAnimationFrames(int start, int duration)
{
	m_modelNode->setFrameLoop(start, start + duration);
}

void EndCallback::OnAnimationEnd(irr::scene::IAnimatedMeshSceneNode *node)
{
	node->setMD2Animation(m_nextAnim.c_str());
}