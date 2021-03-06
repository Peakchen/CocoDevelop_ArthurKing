#pragma once
#include "cocos2d.h"
#include "ArthurKing.h"
#include "MacroDefine.h"
#include "PersonMessageEventHandle.h"

USING_NS_CC;

class CGameMainScene;


class CArthurKingControl :
	public Layer,
	public IAIPersonMessageSink
{
public:
	CArthurKingControl();
	~CArthurKingControl();
	
	void InitData(std::vector<int> vecRowGo, std::vector<int> vecColGo, CActorBase* pActor);
	void StartActorGo();
	virtual bool init();

	void CheckAddAnimateByName(EPLAYER_ACTION iCurAction, std::string  szName, CActorBase* pActor);

	CREATE_FUNC(CArthurKingControl);

	static CArthurKingControl* getActorCtrlInstance();

	CC_SYNTHESIZE(std::vector<int>, m_passColPath, RecordPassColPath);
	CC_SYNTHESIZE(std::vector<int>, m_passRowPath, RecordPassRowPath);

	////////////////////////////////IAIPersonMessageSink/////////////////////////////
	virtual void  OnExecMessageHandle(GWORD nMsgID, const char* szDesc);

	virtual void OnExecRuleEventMessage(int iPDBID, char* szContext, GWORD nMsgID, const char* szDesc){}
private:
	//四方向动画
	Animate* m_pActor_Left_Animate;
	Animate* m_pActor_Right_Animate;
	Animate* m_pActor_Up_Animate;
	Animate* m_pActor_Down_Animate;

	struct TMovePathData
	{
		int iCurCol;
		int iCurRow;
		int iNextCol;
		int iNextRow;

		int iStepCount;
	};

	TMovePathData stPlayerMovePath;

	CallFunc* pCallFunc_MoveEnd;

	CActorBase* m_pActor;

	// 是否 略过下一步摇骰子
	bool  m_bThroughCard;

private:
	void OnPlayerMoveEnd();
	void OnPlayerMove();

	void FindNextPlayer();

	void CreateThreadChechSplitAction();
	// 判断当前角色 暂停次数
	bool __IsPersonStopTimes();
};


