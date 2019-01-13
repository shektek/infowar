#ifndef ANIMATIONCONTROLLER_H
#define ANIMATIONCONTROLLER_H

#include <string>
#include "irrlicht.h"

enum EndBehaviour
{
	ANIM_PAUSE = 0,
	ANIM_LOOP,
	ANIM_CHANGE
};

class EndCallback : public irr::scene::IAnimationEndCallBack
{
private:
	std::string m_nextAnim;

public:
	EndCallback(){}

	void SetNextAnimation(std::string nextAnim) { m_nextAnim = nextAnim; }
	void OnAnimationEnd(irr::scene::IAnimatedMeshSceneNode *node);
};

class AnimationController
{
	public:
		virtual void Init(irr::scene::IAnimatedMeshSceneNode *modelNode) = 0;

		virtual std::string GetCurrentAnimation() = 0;
		virtual void SetAnimation(std::string animName, int fps, EndBehaviour onEnd, std::string switchToThis = "") = 0;
		virtual void SetAnimationFrames(int start, int duration) = 0;
};

class MD2AnimationController : public AnimationController
{
	private:
		EndCallback	*m_endCallback;
		irr::scene::IAnimatedMeshSceneNode *m_modelNode;
		std::string	m_curAnim;

	public:
		MD2AnimationController();
		MD2AnimationController(irr::scene::IAnimatedMeshSceneNode *modelNode);
		~MD2AnimationController();

		void Init(irr::scene::IAnimatedMeshSceneNode *modelNode);

		std::string GetCurrentAnimation() { return m_curAnim; }
		void SetAnimation(std::string animName, int fps, EndBehaviour onEnd, std::string switchToThis = "");
		void SetAnimationFrames(int start, int duration);
};

#endif
