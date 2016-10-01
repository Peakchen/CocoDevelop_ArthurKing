#include "ArthurKingControl.h"
#include "ArthurKing.h"
#include "ConstUtil.h"


CArthurKingControl::CArthurKingControl()
{
}


CArthurKingControl::~CArthurKingControl()
{
}

bool CArthurKingControl::init()
{
	if (!Layer::init())
	{
		return false;
	}


	pCallFunc_MoveEnd = CallFunc::create(CC_CALLBACK_0(CArthurKingControl::OnPlayerMoveEnd, this));
	return true;
}

void CArthurKingControl::OnPlayerMoveEnd()
{
	stPlayerMovePath.iStepCount++;
	if (stPlayerMovePath.iStepCount >= (m_passRowPath.size() - 1))
	{
		return;
	}

	stPlayerMovePath.iCurCol = stPlayerMovePath.iNextCol;
	stPlayerMovePath.iCurRow = stPlayerMovePath.iNextRow;
	OnPlayerMove();
}

void CArthurKingControl::OnPlayerMove()
{
	stPlayerMovePath.iNextCol = m_passColPath[stPlayerMovePath.iStepCount + 1];
	stPlayerMovePath.iNextRow = m_passRowPath[stPlayerMovePath.iStepCount + 1];

	int iHorizontalDistence = stPlayerMovePath.iNextRow - stPlayerMovePath.iCurRow;
	int iVerticalDistence = stPlayerMovePath.iNextCol - stPlayerMovePath.iCurCol;

	MoveBy *pMove = NULL;
	Repeat *pRepeat = NULL;

	CCLOG("iNextCol: %d,  iCurCol: %d", stPlayerMovePath.iNextCol, stPlayerMovePath.iCurCol);
	CCLOG("iNextRow: %d,  iCurRow: %d", stPlayerMovePath.iNextRow, stPlayerMovePath.iCurRow);

	CCLOG("iHorizontalDistence: %d, ", iHorizontalDistence);
	CCLOG("iVerticalDistence: %d, ", iVerticalDistence);

	if (iHorizontalDistence > 0)
	{
		pMove = MoveBy::create(0.4f, ccp(0, TILE_HEIGHT));
		if (m_pActor_Up_Animate != NULL)
		{
			pRepeat = Repeat::create(m_pActor_Up_Animate, 1);
		}

	}
	else if (iHorizontalDistence < 0)
	{
		pMove = MoveBy::create(0.4f, ccp(0, -TILE_HEIGHT));
		if (m_pActor_Down_Animate != NULL)
		{
			pRepeat = Repeat::create(m_pActor_Down_Animate, 1);
		}
	}
	else if (iVerticalDistence > 0)
	{
		pMove = MoveBy::create(0.4f, ccp(TILE_HEIGHT, 0));
		if (m_pActor_Right_Animate != NULL)
		{
			pRepeat = Repeat::create(m_pActor_Right_Animate, 1);
		}
	}
	else if (iVerticalDistence < 0)
	{
		pMove = MoveBy::create(0.4f, ccp(-TILE_HEIGHT, 0));
		if (m_pActor_Left_Animate != NULL)
		{
			pRepeat = Repeat::create(m_pActor_Left_Animate, 1);
		}
	}

	if (pMove == NULL || pRepeat == NULL)
	{
		CCLOG("error: %s 走路动作创建失败...", __FUNCTION__);
		return;
	}
	Spawn* pSpawn = Spawn::create(pMove, pRepeat, NULL);

	if (pSpawn == NULL)
	{
		CCLOG("error: %s 动作产出 失败。。。", __FUNCTION__);
	}
	Sequence* pSequence = Sequence::create(pSpawn, pCallFunc_MoveEnd, NULL);
	m_pActor->runAction(pSequence);
}

void CArthurKingControl::StartActorGo(std::vector<int> vecRowGo, std::vector<int> vecColGo, CArthurKing* pActor)
{
	AnimationCache* pAnimationInstance = AnimationCache::getInstance();
	if (pAnimationInstance == NULL)
	{
		CCLOG("error: %s 取得animation  实例出错。", __FUNCTION__);
		return;
	}

	// 初始化 动画帧
	if (!pAnimationInstance->animationByName("EPlayer_Animation_Left"))
	{
		Animation* pLeftAnimation = Animation::createWithSpriteFrames(pActor->getvecAnim_Left(), 0.1f);
		if (pLeftAnimation == NULL)
		{
			CCLOG("error: %s 取得pLeftAnimation  实例出错。", __FUNCTION__);
			return;
		}
		pAnimationInstance->addAnimation(pLeftAnimation, "EPlayer_Animation_Left");
	}

	if (!pAnimationInstance->animationByName("EPlayer_Animation_Right"))
	{
		Animation* pRightAnimation = Animation::createWithSpriteFrames(pActor->getvecAnim_Right(), 0.1f);
		if (pRightAnimation == NULL)
		{
			CCLOG("error: %s 取得pRightAnimation  实例出错。", __FUNCTION__);
			return;
		}
		pAnimationInstance->addAnimation(pRightAnimation, "EPlayer_Animation_Right");
	}

	if (!pAnimationInstance->animationByName("EPlayer_Animation_UP"))
	{
		Animation* pUpAnimation = Animation::createWithSpriteFrames(pActor->getvecAnim_up(), 0.1f);
		if (pUpAnimation == NULL)
		{
			CCLOG("error: %s 取得pUpAnimation  实例出错。", __FUNCTION__);
			return;
		}
		pAnimationInstance->addAnimation(pUpAnimation, "EPlayer_Animation_UP");
	}

	if (!pAnimationInstance->animationByName("EPlayer_Animation_Down"))
	{
		Animation* pDownAnimation = Animation::createWithSpriteFrames(pActor->getvecAnim_down(), 0.1f);
		if (pDownAnimation == NULL)
		{
			CCLOG("error: %s 取得pDownAnimation  实例出错。", __FUNCTION__);
			return;
		}
		pAnimationInstance->addAnimation(pDownAnimation, "EPlayer_Animation_Down");
	}

	m_pActor_Up_Animate = Animate::create(pAnimationInstance->animationByName("EPlayer_Animation_UP"));
	m_pActor_Down_Animate = Animate::create(pAnimationInstance->animationByName("EPlayer_Animation_Down"));
	m_pActor_Right_Animate = Animate::create(pAnimationInstance->animationByName("EPlayer_Animation_Right"));
	m_pActor_Left_Animate = Animate::create(pAnimationInstance->animationByName("EPlayer_Animation_Left"));

	m_pActor_Left_Animate->retain();
	m_pActor_Right_Animate->retain();
	m_pActor_Up_Animate->retain();
	m_pActor_Down_Animate->retain();

	m_passColPath = vecColGo;
	m_passRowPath = vecRowGo;
	m_pActor = pActor;

	stPlayerMovePath.iCurCol = vecColGo[0];
	stPlayerMovePath.iCurRow = vecRowGo[0];

	stPlayerMovePath.iNextCol = 0;
	stPlayerMovePath.iNextRow = 0;
	stPlayerMovePath.iStepCount = 0;

	OnPlayerMove();
}
