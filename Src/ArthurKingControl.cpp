#include "ArthurKingControl.h"
#include "ArthurKing.h"
#include "ConstUtil.h"
#include "ResCreator.h"
#include "PlayerManager.h"
#include "Entity\PersonPart.h"
#include "EntityProp.h"


CArthurKingControl::CArthurKingControl()
{
	m_pActor_Left_Animate = NULL;
	m_pActor_Right_Animate = NULL;
	m_pActor_Up_Animate = NULL;
	m_pActor_Down_Animate = NULL;

	m_bThroughCard = false;
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

	g_ResCreator.GetPersonMessageInstance()->RegisterAIMessage(DIALOG_CLOSE_ACTION, this, "Close dialog.");
	//pCallFunc_MoveEnd = CallFunc::create(CC_CALLBACK_0(CArthurKingControl::OnPlayerMoveEnd, this));
	return true;
}

void CArthurKingControl::OnPlayerMoveEnd()
{
	stPlayerMovePath.iStepCount++;
	if (stPlayerMovePath.iStepCount >= (m_passRowPath.size() - 1))
	{
		//处理消息
		CreateThreadChechSplitAction();

		// 下一步之前，先处理当前规则属性
		g_ResCreator.GetPersonMessageInstance()->FireMessage(EN_RULE_PROP_EVENT, "规则属性事件");
		
		//if (!m_bThroughCard)
			FindNextPlayer();

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

	/*CCLOG("iNextCol: %d,  iCurCol: %d", stPlayerMovePath.iNextCol, stPlayerMovePath.iCurCol);
	CCLOG("iNextRow: %d,  iCurRow: %d", stPlayerMovePath.iNextRow, stPlayerMovePath.iCurRow);

	CCLOG("iHorizontalDistence: %d, ", iHorizontalDistence);
	CCLOG("iVerticalDistence: %d, ", iVerticalDistence);*/

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

	pCallFunc_MoveEnd = CallFunc::create(CC_CALLBACK_0(CArthurKingControl::OnPlayerMoveEnd, this));

	Sequence* pSequence = Sequence::create(pSpawn,
										   pCallFunc_MoveEnd,
										   NULL);

	m_pActor->runAction(pSequence);
}

void CArthurKingControl::InitData(std::vector<int> vecRowGo, std::vector<int> vecColGo, CActorBase* pActor)
{
	m_pActor = NULL;


	EPLAYER_ACTION iCurAction = g_PalyerManager.getRecordNextPlayerAction();

	// 初始化 动画帧
	AnimationCache* pAnimationInstance = AnimationCache::getInstance();
	if (pAnimationInstance == NULL)
	{
		CCLOG("error: %s 取得animation  实例出错。", __FUNCTION__);
		return;
	}
	if (iCurAction == EACTOR_PLAYER_ACTION)
	{
		CheckAddAnimateByName(EACTOR_PLAYER_ACTION, "EPlayer", pActor);
	}
	else if (iCurAction == EAI_PLAYER_ACTION)
	{
		CheckAddAnimateByName(EAI_PLAYER_ACTION, "EAI", pActor);
	}

	m_pActor_Left_Animate->retain();
	m_pActor_Right_Animate->retain();
	m_pActor_Up_Animate->retain();
	m_pActor_Down_Animate->retain();

	g_ResCreator.GetActorCtrlInstance()->setRecordPassColPath(vecColGo);
	g_ResCreator.GetActorCtrlInstance()->setRecordPassRowPath(vecRowGo);

	m_passColPath = vecColGo;
	m_passRowPath = vecRowGo;
	m_pActor = pActor;

	stPlayerMovePath.iCurCol = vecColGo[0];
	stPlayerMovePath.iCurRow = vecRowGo[0];

	stPlayerMovePath.iNextCol = 0;
	stPlayerMovePath.iNextRow = 0;
	stPlayerMovePath.iStepCount = 0;

}

void CArthurKingControl::StartActorGo()
{
	OnPlayerMove();
}

CArthurKingControl* CArthurKingControl::getActorCtrlInstance()
{
	CArthurKingControl* pCtrl = new CArthurKingControl();
	return pCtrl;
}

void CArthurKingControl::CheckAddAnimateByName(EPLAYER_ACTION iCurAction, std::string szName, CActorBase* pActor)
{
	AnimationCache* pAnimationInstance = AnimationCache::getInstance();
	if (pAnimationInstance == NULL)
	{
		CCLOG("error: %s 取得animation  实例出错。", __FUNCTION__);
		return;
	}

	if (!pAnimationInstance->animationByName(szName + "_Animation_Left"))
	{
		Animation* pLeftAnimation = Animation::createWithSpriteFrames(pActor->getvecAnim_Left(), 0.1f);
		if (pLeftAnimation == NULL)
		{
			CCLOG("error: %s 取得pLeftAnimation  实例出错。", __FUNCTION__);
			return;
		}
		pAnimationInstance->addAnimation(pLeftAnimation, szName + "_Animation_Left");
	}

	if (!pAnimationInstance->animationByName(szName + "_Animation_Right"))
	{
		Animation* pRightAnimation = Animation::createWithSpriteFrames(pActor->getvecAnim_Right(), 0.1f);
		if (pRightAnimation == NULL)
		{
			CCLOG("error: %s 取得pRightAnimation  实例出错。", __FUNCTION__);
			return;
		}
		pAnimationInstance->addAnimation(pRightAnimation, szName + "_Animation_Right");
	}

	if (!pAnimationInstance->animationByName(szName + "_Animation_UP"))
	{
		Animation* pUpAnimation = Animation::createWithSpriteFrames(pActor->getvecAnim_up(), 0.1f);
		if (pUpAnimation == NULL)
		{
			CCLOG("error: %s 取得pUpAnimation  实例出错。", __FUNCTION__);
			return;
		}
		pAnimationInstance->addAnimation(pUpAnimation, szName + "_Animation_UP");
	}

	if (!pAnimationInstance->animationByName(szName + "_Animation_Down"))
	{
		Animation* pDownAnimation = Animation::createWithSpriteFrames(pActor->getvecAnim_down(), 0.1f);
		if (pDownAnimation == NULL)
		{
			CCLOG("error: %s 取得pDownAnimation  实例出错。", __FUNCTION__);
			return;
		}
		pAnimationInstance->addAnimation(pDownAnimation, szName + "_Animation_Down");
	}

	m_pActor_Up_Animate = Animate::create(pAnimationInstance->animationByName(szName + "_Animation_UP"));
	m_pActor_Down_Animate = Animate::create(pAnimationInstance->animationByName(szName + "_Animation_Down"));
	m_pActor_Right_Animate = Animate::create(pAnimationInstance->animationByName(szName + "_Animation_Right"));
	m_pActor_Left_Animate = Animate::create(pAnimationInstance->animationByName(szName + "_Animation_Left"));

}

void CArthurKingControl::FindNextPlayer()
{
	// 当前进行下一步
	g_PalyerManager.SetFinallyStepPerson(m_pActor);
	EPLAYER_ACTION iCurAction = g_PalyerManager.getRecordNextPlayerAction();
	g_PalyerManager.DoChangeState(iCurAction);
	int iNextAction = ( iCurAction % Player_Max_Count );
	iNextAction += 1;

	g_PalyerManager.setRecordCurPlayerAction(iCurAction);
	g_PalyerManager.setRecordNextPlayerAction(( EPLAYER_ACTION ) iNextAction);
}

void CArthurKingControl::OnExecMessageHandle(GWORD nMsgID, const char* szDesc)
{
	switch (nMsgID)
	{
		case DIALOG_CLOSE_ACTION:
			{
				//CCLOG("DIALOG_CLOSE_ACTION  file: %s is wrong, function: %s...", __LINE__, __FUNCTION__);
				FindNextPlayer();
			}
			break;
		default:
			CCLOG("file: %s is wrong...", __LINE__, __FUNCTION__);
			break;
	}
}

void CArthurKingControl::CreateThreadChechSplitAction()
{
	float iRow = m_passRowPath.back();
	float iCol = m_passColPath.back();

	float y = ( float )1.0f*iRow * TILE_WIDTH;
	float x = ( float )1.0f*iCol * TILE_HEIGHT;

	Vec2 opoint = Vec2(x, y);
	CCLOG("%s src Point: x=%02f, y=%02f", __FUNCTION__, x, y);
	if (!g_PalyerManager.CheckActionSplit(opoint, m_pActor, false))
	{
		CCLOG("action split is faild....");
		m_bThroughCard = false;
		return;
	}

	// 略过，因为 弹窗
	m_bThroughCard = true;
}

bool CArthurKingControl::__IsPersonStopTimes()
{
	int iTimes = g_PersonPart.GetPersonRuleProp(m_pActor->GetPDBID(), CREATURE_RULE_FOOT_RED);
	if (iTimes > 0)
	{
		iTimes -= 1;
		g_PersonPart.SetPersonRuleProp(m_pActor->GetPDBID(), CREATURE_RULE_FOOT_RED, iTimes);
		return true;
	}

	return false;
}
