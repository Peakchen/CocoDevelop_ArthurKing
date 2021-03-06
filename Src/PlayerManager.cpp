#include "PlayerManager.h"
#include "ConstUtil.h"
#include "ResCreator.h"
#include "ActionSpiltManager.h"
#include "EntityProp.h"
#include "Entity\PersonPart.h"

CPlayerManager g_PalyerManager;

void CPlayerManager::DoChangeState(EPLAYER_ACTION nState)
{
	if (m_pActorBase != NULL)
	{
		m_pActorBase->OnLeave();
	}

	switch (nState)
	{
		case EACTOR_PLAYER_ACTION:
			{
				m_pActorBase = &m_oArthur;
			}
			break;
		case EAI_PLAYER_ACTION:
			{
				m_pActorBase = &m_oAiPlayer;
			}
			break;
		default:
			CCLOG("error: %s action is wrong...", __FUNCTION__);
			break;
	}

	if (m_pActorBase)
	{
		m_pActorBase->OnEnter();
	}
}

void CPlayerManager::Release()
{
	CC_SAFE_DELETE(m_pActorBase);
	m_pActorBase = NULL;
}

void CPlayerManager::Create(CActorBase* pActor)
{
	m_pActorBase = pActor;

	m_CurPlayerAction = EACTOR_PLAYER_ACTION;
	m_NextPlayerAction = EAI_PLAYER_ACTION;

	m_mapActorTileLayerInfo.clear();

}

CPlayerManager::CPlayerManager()
{
	m_pActorBase = nullptr;
	m_pCurrentActor = nullptr;
}

CPlayerManager::~CPlayerManager()
{

}

void CPlayerManager::LoadPersonLocationInfoData(std::string filename)
{
	m_mapPersonLocation.clear();

	// judge file is existed?
	std::string stFile = FileUtils::getInstance()->getStringFromFile(filename);
	if (stFile.empty())
	{
		Trace_In("warning: %s file is not exist...", __FUNCTION__);
		return;
	}

	ValueMap stFileMap = FileUtils::getInstance()->getValueMapFromFile(filename);
	ValueMap::iterator itBegin = stFileMap.begin();
	for (; itBegin != stFileMap.end(); ++itBegin)
	{
		TPersonInfo stPerson;

		stPerson.dt_score		= stFileMap ["score"].asFloat();
		stPerson.dw_save_time	= stFileMap ["save_time"].asDouble();
		stPerson.Loc_x			= stFileMap ["x"].asFloat();
		stPerson.Loc_y			= stFileMap ["y"].asFloat();
		stPerson.person_ID		= stFileMap ["PDBID"].asFloat();

		m_mapPersonLocation [stPerson.person_ID] = &stPerson;
	}
}

void CPlayerManager::SavePersonLocationInfoData()
{
	if (m_mapPersonLocation.size() == 0)
	{
		Trace_In("error: %s map data is empty...", __FUNCTION__);
		return;
	}

	Json::Value stArrayList;

	TPersonInfoMap::iterator itBegin = m_mapPersonLocation.begin();
	for (; itBegin != m_mapPersonLocation.end(); ++itBegin)
	{
		TPersonInfo* stPerson = itBegin->second;

		Json::Value stValue;
		stValue ["score"]		= stPerson->dt_score;
		stValue ["save_time"]	= stPerson->dw_save_time;
		stValue ["x"]			= stPerson->Loc_x;
		stValue ["y"]			= stPerson->Loc_y;
		stValue ["PDBID"]		= stPerson->person_ID;

		stArrayList.append(stValue);
	}


	Json::FastWriter owriter;
	string json_file = owriter.write(stArrayList);

	string strPath = "";
#if DEBUG
	strPath = FileUtils::getInstance()->fullPathForFilename(PERSON_JSON_FILE);
#else
	strPath = FileUtils::getInstance()->getWritablePath();
	strPath.append(PERSON_JSON_FILE);
#endif

	FILE* pfile = fopen(strPath.c_str(), "wb");
	if (pfile)
	{
		fputs(json_file.c_str(), pfile);
		fclose(pfile);
	}

}

TPersonInfo* CPlayerManager::GetPersonInfo(__int8 PDBID)
{
	if (m_mapPersonLocation [PDBID] == NULL)
		return NULL; 
	
	return m_mapPersonLocation [PDBID];

}

void CPlayerManager::AddActorInstace(CActorBase* pActor)
{
	assert(pActor != NULL);
	m_mapActorTileLayerInfo [pActor->GetPDBID()] = pActor;
}

CActorBase* CPlayerManager::GetActorInstace(__int8 PDBID)
{
	if (PDBID <= 0)
		return NULL;
	return m_mapActorTileLayerInfo [PDBID];
}

bool CPlayerManager::CheckCanTakeAddSocre(GWORD &dwScore, Vec2 oPoint)
{

	//CCLOG("Dest: x = %02f,   y = %02f ", oPoint.x, oPoint.y);
	ValueVector vecArray = g_ResCreator.GetMapReaderInstance()->getVecObjectPath();

	ValueVector::iterator itBegin = vecArray.begin();
	for (; itBegin != vecArray.end(); ++itBegin)
	{
		ValueMap  mapObject = itBegin->asValueMap();

		//CCLOG("value: %d, %s", mapObject.size(), mapObject ["name"]);
		float x = mapObject ["x"].asFloat();
		float y = mapObject ["y"].asFloat();

		Vec2 vecPoint = Vec2(x, y);

		Vec2 vecPoint1 = Vec2(x + 1, y + 1);
		Vec2 vecPoint2 = Vec2(x + 1, y);
		Vec2 vecPoint3 = Vec2(x, y + 1);

		Vec2 vecPoint4 = Vec2(x - 1, y - 1);
		Vec2 vecPoint5 = Vec2(x - 1, y);
		Vec2 vecPoint6 = Vec2(x, y - 1);

		//CCLOG("Src: x = %02f,   y = %02f ", x, y);

		bool bFlag_x = ( fabs(oPoint.x - x) <= CoordinateDiff );
		bool bFlag_y = ( fabs(oPoint.y - y) <= CoordinateDiff );
		//CCLOG("fabs: x = %02f,   y = %02f ", fabs(oPoint.x - x), fabs(oPoint.y - y));

		if (bFlag_x && bFlag_y)
		{
			if (DoParseScore(mapObject, dwScore))
			{
				/*CCLOG("Src Map		value: %d,	name: %s", mapObject.size(), mapObject ["name"]);
				CCLOG("Src Map		X: %s,		Y: %s", mapObject ["x"].asString().c_str(), mapObject ["y"].asString().c_str());
				CCLOG("Src eMap		Type: %s", mapObject ["Type"].asString().c_str());
				CCLOG("Src: x = %02f,   y = %02f ", x, y);*/
				return true;
			}
		}

	}
	return false;
}

bool CPlayerManager::DoParseScore(ValueMap mapObject, GWORD &dwScore)
{
	if (strcmp(mapObject ["name"].asString().c_str(), "Score") == 0)
	{
		CCLOG("find Score pos....");

		if (strcmp(mapObject ["Type"].asString().c_str(), "int") == 0)
		{
			/*CCLOG("---------------------------finally find result Coordinate----------------------------");
			CCLOG("Dest Map		value: %d,	name: %s", mapObject.size(), mapObject ["name"]);
			CCLOG("Dest Map		X: %s,		Y: %s", mapObject ["x"].asString().c_str(), mapObject ["y"].asString().c_str());
			CCLOG("Dest Map			Type: %s", mapObject ["Type"].asString().c_str());
			CCLOG("Dest Map			Type: %s", mapObject ["Value"].asString().c_str());*/

			dwScore = mapObject ["Value"].asDouble();

			return true;
		}
	}

	return false;
}

bool CPlayerManager::CheckActionSplit(Vec2 opint, CActorBase* pActor, bool bCallback /* = false */, TTileLayerGridProperty** pTileGrid /* = nullptr*/)
{
	CCLOG("func:%s  Dest: x = %02f,   y = %02f ", __FUNCTION__, opint.x, opint.y);
	ValueVector vecArray = g_ResCreator.GetMapReaderInstance()->getVecObjectPath();

	ValueVector::iterator itBegin = vecArray.begin();
	for (; itBegin != vecArray.end(); ++itBegin)
	{
		ValueMap  mapObject = itBegin->asValueMap();

		//CCLOG("value: %d, %s", mapObject.size(), mapObject ["name"]);
		float x = mapObject ["x"].asFloat();
		float y = mapObject ["y"].asFloat();

		Vec2 vecPoint = Vec2(x, y);

		Vec2 vecPoint1 = Vec2(x + 1, y + 1);
		Vec2 vecPoint2 = Vec2(x + 1, y);
		Vec2 vecPoint3 = Vec2(x, y + 1);

		Vec2 vecPoint4 = Vec2(x - 1, y - 1);
		Vec2 vecPoint5 = Vec2(x - 1, y);
		Vec2 vecPoint6 = Vec2(x, y - 1);

		//CCLOG("Src: x = %02f,   y = %02f ", x, y);
		bool bFlag_x = ( fabs(opint.x - x) <= CoordinateDiff );
		bool bFlag_y = ( fabs(opint.y - y) <= CoordinateDiff );
		CCLOG("fabs: x = %02f,   y = %02f ", fabs(opint.x - x), fabs(opint.y - y));

		if (bFlag_x && bFlag_y)
		{
			TSPLITHANDLERMAP* pSplitAction = g_ActionSpiltManager.GetSplitHandlerMap();
			if (pSplitAction->empty() || pActor == nullptr)
			{
				return false;
			}

			string szName = mapObject ["name"].asString();
			string szType = mapObject ["Type"].asString();
			TSPLITHANDLERMAP::iterator it = pSplitAction->find(szName);
			if (it != pSplitAction->end())
			{
				TTileLayerGridProperty oTileGridPeperty;
				__GetTileContextByName(szName, &oTileGridPeperty, mapObject);
				RemoveActorInstace(pActor->GetPDBID());

				IArthurActionSpiltHandler* pSpiltHandler = it->second;
				if (!pSpiltHandler)
				{
					return false;
				}

				if (bCallback)
				{
					*pTileGrid = &oTileGridPeperty;
				}
				else
				{
					it->second->CheckCurrentAction(&oTileGridPeperty, pActor, &m_mapActorTileLayerInfo);
				}
				return true;
			}
			else
			{
				CCLOG("no find this location....");
			}
			return false;
		}
	}

	return false;
}

void CPlayerManager::__GetTileContextByName(string szName, TTileLayerGridProperty *pTileContext, ValueMap mapObject)
{
	pTileContext->szName = mapObject ["name"].asString();
	pTileContext->szType = mapObject ["Type"].asString();

	if (szName == ETILELAYER_ONCEAGAIN)
	{
		return;
	}
	else if (szName == ETILELAYER_BLUE_DOUBLESTAR)
	{
		pTileContext->iTimes = mapObject ["times"].asInt();
	}
	else if (szName == ETILELAYER_BLUE_STAR)
	{
		pTileContext->iTimes = mapObject ["times"].asInt();
		pTileContext->iScoreMult = mapObject ["ScoreMult"].asInt();
	}
	else if (szName == ETILELAYER_FOOT_BLUE)
	{
		pTileContext->iStopTimes = mapObject ["stoptimes"].asInt();
	}
	else if (szName == ETILELAYER_FOOT_RED)
	{
		pTileContext->iStopTimes = mapObject ["stoptimes"].asInt();
	}
	else if (szName == ETILELAYER_QUESTION)
	{
		
	}
	else if (szName == ETILELAYER_RED_DOUBLESTAR)
	{
		pTileContext->iScoreMult = mapObject ["ScoreMult"].asInt();
		pTileContext->iTimes = mapObject["times"].asInt();
	}
	else if (szName == ETILELAYER_SCORE)
	{
		pTileContext->iScoreValue = mapObject ["value"].asInt();
	}
	else if (szName == ETILELAYER_SEABAR)
	{
		pTileContext->iSeaBarIndex = mapObject ["index"].asInt();
		pTileContext->iSeaBarPrice = mapObject["price"].asInt();
		pTileContext->iSeaBarTip = mapObject ["tip"].asInt();
	}
	else if (szName == ETILELAYER_TUEN_FREE)
	{
		pTileContext->iTimes = mapObject ["times"].asInt();
	}
	else if (szName == ETILELAYER_YELLOW_STAR)
	{
		pTileContext->iTimes = mapObject ["times"].asInt();
	}
}

void CPlayerManager::RemoveActorInstace(int PDBID)
{
	if (m_mapActorTileLayerInfo.empty())
	{
		return;
	}

	m_mapActorTileLayerInfo.erase(PDBID);
}

void CPlayerManager::LoadPersonRulePropInfoData(std::string filename)
{
	if (m_mapPersonLocation.size() == 0)
	{
		Trace_In("error: %s map data is empty...", __FUNCTION__);
		return;
	}

	// judge file is existed?
	std::string stFile = FileUtils::getInstance()->getStringFromFile(filename);
	if (stFile.empty())
	{
		Trace_In("warning: %s file is not exist...", __FUNCTION__);
		return;
	}

	ValueMap stFileMap = FileUtils::getInstance()->getValueMapFromFile(filename);
	ValueMap::iterator itBegin = stFileMap.begin();
	for (; itBegin != stFileMap.end(); ++itBegin)
	{
		FOR_EACH_CONTAINER(TPersonInfoMap, m_mapPersonLocation, it){
			g_PersonPart.SetPersonRuleProp(it->first, CREATURE_RULE_QUESTION, stFileMap [ETILELAYER_QUESTION].asInt());
			g_PersonPart.SetPersonRuleProp(it->first, CREATURE_RULE_TURN_FREE, stFileMap [ETILELAYER_TUEN_FREE].asInt());
			g_PersonPart.SetPersonRuleProp(it->first, CREATURE_RULE_FOOT_RED, stFileMap [ETILELAYER_FOOT_RED].asInt());
			g_PersonPart.SetPersonRuleProp(it->first, CREATURE_RULE_YELLOW_STAR, stFileMap [ETILELAYER_YELLOW_STAR].asInt());
			g_PersonPart.SetPersonRuleProp(it->first, CREATURE_RULE_RED_DOUBLESTAR, stFileMap [ETILELAYER_RED_DOUBLESTAR].asInt());
			g_PersonPart.SetPersonRuleProp(it->first, CREATURE_RULE_FOOT_BLUE, stFileMap [ETILELAYER_FOOT_BLUE].asInt());
			g_PersonPart.SetPersonRuleProp(it->first, CREATURE_RULE_BLUE_STAR, stFileMap [ETILELAYER_BLUE_STAR].asInt());
			g_PersonPart.SetPersonRuleProp(it->first, CREATURE_RULE_BLUE_DOUBLESTAR, stFileMap [ETILELAYER_BLUE_DOUBLESTAR].asInt());
		}
	}
}

void CPlayerManager::SavePersonRulePropInfoData()
{
	if (m_mapPersonLocation.size() == 0)
	{
		Trace_In("error: %s map data is empty...", __FUNCTION__);
		return;
	}

	Json::Value stArrayList;

	TPersonInfoMap::iterator itBegin = m_mapPersonLocation.begin();
	for (; itBegin != m_mapPersonLocation.end(); ++itBegin)
	{
		Json::Value stValue;

		stValue [ETILELAYER_QUESTION] = g_PersonPart.GetPersonRuleProp(itBegin->first, CREATURE_RULE_QUESTION);
		stValue [ETILELAYER_TUEN_FREE] = g_PersonPart.GetPersonRuleProp(itBegin->first, CREATURE_RULE_TURN_FREE);
		stValue [ETILELAYER_FOOT_RED] = g_PersonPart.GetPersonRuleProp(itBegin->first, CREATURE_RULE_FOOT_RED);
		stValue [ETILELAYER_YELLOW_STAR] = g_PersonPart.GetPersonRuleProp(itBegin->first, CREATURE_RULE_YELLOW_STAR);
		stValue [ETILELAYER_RED_DOUBLESTAR] = g_PersonPart.GetPersonRuleProp(itBegin->first, CREATURE_RULE_RED_DOUBLESTAR);
		stValue [ETILELAYER_FOOT_BLUE] = g_PersonPart.GetPersonRuleProp(itBegin->first, CREATURE_RULE_FOOT_BLUE);
		stValue [ETILELAYER_BLUE_STAR] = g_PersonPart.GetPersonRuleProp(itBegin->first, CREATURE_RULE_BLUE_STAR);
		stValue [ETILELAYER_BLUE_DOUBLESTAR] = g_PersonPart.GetPersonRuleProp(itBegin->first, CREATURE_RULE_BLUE_DOUBLESTAR);

		stArrayList.append(stValue);
	}


	Json::FastWriter owriter;
	string json_file = owriter.write(stArrayList);

	string strPath = "";
#if DEBUG
	strPath = FileUtils::getInstance()->fullPathForFilename(PERSON_JSON_FILE);
#else
	strPath = FileUtils::getInstance()->getWritablePath();
	strPath.append(PERSON_RULE_PROP_FILE);
#endif

	FILE* pfile = fopen(strPath.c_str(), "wb");
	if (pfile)
	{
		fputs(json_file.c_str(), pfile);
		fclose(pfile);
	}

}

void CPlayerManager::LoadPersonPropInfoData(std::string filename)
{
	if (m_mapPersonLocation.size() == 0)
	{
		Trace_In("error: %s map data is empty...", __FUNCTION__);
		return;
	}

	// judge file is existed?
	std::string stFile = FileUtils::getInstance()->getStringFromFile(filename);
	if (stFile.empty())
	{
		Trace_In("warning: %s file is not exist...", __FUNCTION__);
		return;
	}

	ValueMap stFileMap = FileUtils::getInstance()->getValueMapFromFile(filename);
	ValueMap::iterator itBegin = stFileMap.begin();
	for (; itBegin != stFileMap.end(); ++itBegin)
	{
		FOR_EACH_CONTAINER(TPersonInfoMap, m_mapPersonLocation, it){
			g_PersonPart.SetPersonRuleProp(it->first, CREATURE_PROP_PDBID, stFileMap ["PDBID"].asInt());
		}
	}
}

void CPlayerManager::SavePersonPropInfoData()
{
	if (m_mapPersonLocation.size() == 0)
	{
		Trace_In("error: %s map data is empty...", __FUNCTION__);
		return;
	}

	Json::Value stArrayList;

	TPersonInfoMap::iterator itBegin = m_mapPersonLocation.begin();
	for (; itBegin != m_mapPersonLocation.end(); ++itBegin)
	{
		Json::Value stValue;

		stValue ["PDBID"] = g_PersonPart.GetPersonRuleProp(itBegin->first, CREATURE_PROP_PDBID);

		stArrayList.append(stValue);
	}


	Json::FastWriter owriter;
	string json_file = owriter.write(stArrayList);

	string strPath = "";
#if DEBUG
	strPath = FileUtils::getInstance()->fullPathForFilename(PERSON_JSON_FILE);
#else
	strPath = FileUtils::getInstance()->getWritablePath();
	strPath.append(PERSON_PROP_FILE);
#endif

	FILE* pfile = fopen(strPath.c_str(), "wb");
	if (pfile)
	{
		fputs(json_file.c_str(), pfile);
		fclose(pfile);
	}
}

void CPlayerManager::SetActorLocInfo(TPersonInfo *pLocInfo, int PDBID)
{
	TPersonInfoMap::iterator it = m_mapPersonLocation.find(PDBID);
	if (it != m_mapPersonLocation.end())
	{
		return;
	}

	if (pLocInfo == nullptr || PDBID <= 0)
	{
		return;
	}

	m_mapPersonLocation [PDBID] = pLocInfo;
}

void CPlayerManager::SetFinallyStepPerson(CActorBase *pActor)
{
	if (pActor == nullptr)
	{
		log("%s :获取最后一步角色失败, 为nil.", __FUNCTION__);
	}
	m_pCurrentActor = pActor;
}

CActorBase* CPlayerManager::GetFinallyStepPerson()
{
	return m_pCurrentActor;
}
