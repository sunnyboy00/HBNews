#include "user_task_mgr.h"
bool CUserTaskMgr::UnInit()
{
	return false;
}

bool CUserTaskMgr::IsSubTaskDoneByUser(UINT32 iUserID, UINT32 iSubTaskID){
	map<UINT32, map<UINT32, CUserSubTask*>>::iterator it = mMapUserID2SubTaskID2SubTask.find(iUserID);
	if(it != mMapUserID2SubTaskID2SubTask.end()){
		map<UINT32, CUserSubTask*>::iterator itIn = it->second.find(iSubTaskID);
		if(itIn != it->second.end()){
			CUserSubTask* pUserSubTask = itIn->second;
			if (pUserSubTask)
			{
				return pUserSubTask->iStatus == EUSERSUBTASKSTATUS_DONE;
			}
			//不应该
			//else{
			//logo error!
			//}
		}
	}

	return false;
}

bool CUserTaskMgr::IsTaskDoneByUser( UINT32 iUserID, UINT32 iTaskID )
{
	map<UINT32, map<UINT32, CUserTask*>>::iterator it = mMapUserID2TaskID2UserTask.find(iUserID);
	if(it != mMapUserID2TaskID2UserTask.end()){
		map<UINT32, CUserTask*>::iterator itIn = it->second.find(iTaskID);
		if(itIn != it->second.end()){
			CUserTask* pUserTask = itIn->second;
			if (pUserTask)
			{
				return pUserTask->iStatus == EUSERTASKSTATUS_DONE;
			}
			//不应该
			//else{
			//logo error!
			//}
		}
	}

	return false;
}

bool CUserTaskMgr::CanUserVisitMapPoint( UINT32 iUserID, UINT32 iMapPointID )
{
	//获取玩家最大章节信息
	stUserChapterInfo userChapterInfo = {0};
	EErrorCode ret = GetUserChapterInfo(iUserID, userChapterInfo);
	if (ret != EErrorCode_Success)
	{
		return false;
	}

	UINT16 currentChapter = userChapterInfo.iCurrentChapter;

	//获取据点章节信息
	CMapPoint* pMapPoint = CMapMgr::GetInstance()->GetMapPointByID(iMapPointID);
	if (!pMapPoint)
	{
		//todo log error
		return false;
	}

	if(pMapPoint->iChapterID < currentChapter){
		return true;
	}
	else if (pMapPoint->iChapterID > currentChapter)
	{
		return false;
	}
	else{// 章节相等的时候 判断主线任务副本是否够远

		UINT16 iChapterID = pMapPoint->iChapterID;
		set<UINT32> userCanVisitMapPoints;
		UINT32 curMainTaskPointID;
		GetMapPointUserCanVisit(iUserID, iChapterID, userCanVisitMapPoints, curMainTaskPointID);
		return userCanVisitMapPoints.find(iMapPointID) != userCanVisitMapPoints.end();
	}

	return false;
}

EErrorCode CUserTaskMgr::GetUserChapterInfo( UINT32 iUserID, stUserChapterInfo& userChapterInfo)
{
	userChapterInfo.iCurrentChapter = 1;
	userChapterInfo.pCurrentMainTask = NULL;

	map<UINT16, vector<CTask*>>& chapterMainTask = CTaskMgr::GetInstance()->mMapChapterID2MainTaskList;

	//todo 获取当前的最大章节！
	while (true)
	{
		map<UINT16, vector<CTask*>>::iterator it = chapterMainTask.find(userChapterInfo.iCurrentChapter);
		if (it != chapterMainTask.end())
		{
			vector<CTask*> & mainTaskList  = it->second;

			for (vector<CTask*>::iterator itIn = mainTaskList.begin() ; itIn != mainTaskList.end() ; ++itIn)
			{	
				//todo 判断 *itIn 是否为空
				CTask * pTask  = (*itIn);
				if (!pTask)
				{
					//log error！
					//return 0;
					return EErrorCode_TASKCONFIG_ERROR;
				}

				if(!IsTaskDoneByUser(iUserID, pTask->iTaskID)){
					//return currentChapter;
					userChapterInfo.pCurrentMainTask = pTask;
					goto END;
				}
			}

		}
		else{//说明这一章节的主线任务都过了 
			break;
		}

		++userChapterInfo.iCurrentChapter;//注意 它可能为当前策划配置的最大章节 + 1！
	}

END:
	
	return EErrorCode_Success;
}

//retPoints 返回可以访问的据点id集合
//curMainTaskPointID 下一个主线任务所在的地图ID
bool CUserTaskMgr::GetMapPointUserCanVisit( UINT32 iUserID, UINT16 iChapterID, set<UINT32> &retPoints, UINT32& curMainTaskPointID)
{
	//static vector<UINT32> nullMapPointUserCanVisit;

	//set<UINT32> setPointsCanbVisitTemp;

	retPoints.clear();
	curMainTaskPointID = 0;

	//map<UINT32, bool> setPointsCanbVisitTemp;//不用set是因为 后面求连接点时还需要用标记记录是否判断过

	/*
	*/

	//1. 初始就可见的点  怎么来？
	//2. 玩家已经通过主线任务的地图点
	//3. 玩家已经通过主线任务的地图点的连接点


	//1. 初始点
	//GetUserMaxChapter计算量很大 给外部使用的， 内部还是自己根据任务计算吧！

	//获取玩家最大章节信息
	stUserChapterInfo userChapterInfo = {0};
	EErrorCode ret = GetUserChapterInfo(iUserID, userChapterInfo);
	if (ret != EErrorCode_Success)
	{
		return false;
	}

	if (iChapterID > userChapterInfo.iCurrentChapter)
	{
		return false;
	}

	curMainTaskPointID = CTaskMgr::GetInstance()->GetMapPointIDByTaskID(userChapterInfo.pCurrentMainTask->iTaskID);

	UINT32 bornMapPointID = GetChapterBornMapPointID(iChapterID);
	retPoints.insert(bornMapPointID);//初始ID


	//2. 已通过主线任务的地图据点
	CTaskMgr* gpTaskMgr = CTaskMgr::GetInstance(); 

	{
		map<UINT32, map<UINT32, CUserTask*>>::iterator it = mMapUserID2TaskID2UserTask.find(iUserID);
		if (it == mMapUserID2TaskID2UserTask.end() || it->second.empty())
		{
			//todo 是否有问题？
			return true;
		}

		map<UINT32, CUserTask*>& userTaskList = it->second;
		for (map<UINT32, CUserTask*>::iterator itIn = userTaskList.begin() ; itIn != userTaskList.end() ; ++itIn)
		{
			CUserTask * pUserTask = itIn->second;
			if(!pUserTask){//玩家任务不存在
				//todo log error!
				//continue;
				return false;
			}

			UINT16 iUserTaskChapterID = CTaskMgr::GetInstance()->GetChapterIDByTaskID(pUserTask->iTaskID);
			if(iUserTaskChapterID != iChapterID){//不是同一个章节的任务！
				continue;
			}

			CTask* pTask = gpTaskMgr->GetTaskByTaskID(pUserTask->iTaskID);
			if(!pTask){//任务配置不存在
				//todo log error! 放在GetTaskByTaskID里面
				//continue;
				return false;

			}

			//主线任务决定地图据点可访问！
			if (pTask->iType == ETASKTYPE_MAIN
				&& pUserTask->iStatus == EUSERTASKSTATUS_DONE)
			{
				retPoints.insert(pTask->iMapPointID);//set防止  一个点上多个主线
			}
		}
	}


	//3. 获取1,2的一级临近点
	queue<UINT32> qMapPoint;
	set<UINT32>::iterator itMapPoint = retPoints.begin();
	while ( itMapPoint != retPoints.end())
	{
		qMapPoint.push(*itMapPoint);
		++itMapPoint;
	}

	while (!qMapPoint.empty())
	{
		UINT32 iMapPointID = qMapPoint.front();
		qMapPoint.pop();
		//获取临近的点
		CMapPoint* pMapPoint = CMapMgr::GetInstance()->GetMapPointByID(iMapPointID);
		for (int i = 0 ; i < MAXNUM_JOINEDMAPPOINT ; ++i)
		{
			UINT32 iJoinedMapPointID = pMapPoint->arrJoinedMapPointID[i];
			if(iJoinedMapPointID){
				if(CMapMgr::GetInstance()->GetMapPointByID(iJoinedMapPointID)){//验证存在这个地图据点！ 实际上这个工作应该在读出的时候做 但是这里多做一下保险！
					retPoints.insert(iJoinedMapPointID);
				}
				else{
					//todo log error!
					return false;
				}
			}
		}
	}

	return true;
}

void CUserTaskMgr::OnGS_C_Req_Open_Chapter_Map( UINT32 iUserID, UINT16 iChapterID, GS_C_Res_Open_Chapter_Map& st )
{
	set<UINT32> userCanVisitMapPoints;
	UINT32 curMainTaskPointID;
	if(!GetMapPointUserCanVisit(iUserID, iChapterID, userCanVisitMapPoints, curMainTaskPointID)){
		st.set_error_code(EErrorCode_UnKown_Error);
		return ;
	}

	st.set_cur_main_task_map_point(curMainTaskPointID);
	for (set<UINT32>::iterator it = userCanVisitMapPoints.begin() ; it != userCanVisitMapPoints.end() ; ++it)
	{
		UINT32 iMapPointID = *it;
		struct_map_point_info* ele = st.add_map_point_info();
		ele->set_map_point_id(iMapPointID);
		//ele->set_not_done_branch_task(0);
		//计算未完成的任务数
		vector<CTask*> taskList;
		UINT32 notDoneNum = 0;
		CTaskMgr::GetInstance()->GetTaskListByMapPointID(iMapPointID, taskList);
		for (vector<CTask*>::iterator itTask = taskList.begin(); itTask != taskList.end() ; ++itTask)
		{
			assert(*itTask);
			if(!IsTaskDoneByUser(iUserID, (*itTask)->iTaskID)){
				++notDoneNum;
			}
		}
		ele->set_not_done_branch_task(notDoneNum);
	}

	st.set_error_code(EErrorCode_Success);
}