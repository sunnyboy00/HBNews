#include <string>
#include <vector>
#include "fight_mgr.h"
#include "db_mgr.h"
#include "db_sql.h"
#include "str.h"
stBattle* CFightMgr::GetBattleByID( UINT32 iBattleID )
{
	std::map<UINT32, stBattle*>::iterator it = m_mapBattleID2Battle.find(iBattleID);
	if (it != m_mapBattleID2Battle.end())
	{
		return it->second;
	}
	return NULL;
}

bool CFightMgr::Init()
{
	MYSQL* con = CDBMgr::GetInstance()->GetConnection();

	char* sql = SELECT_BATTLE;
	int code = CDBMgr::Query(con, sql, (unsigned int)strlen(sql));
	if (0 != code)   //非0查询失败  
	{
		printf("query failed! [%s]\n", sql);  
		return false;  
	}

	MYSQL_RES* res = mysql_store_result(con);   //保存查询结果  

	//检查表结构是否有改变
	//std::string need_fields[] = {"id", "battle_id", "scene_objects_type_subtype_id_pos_arr"};
	std::string need_fields[] = {"battle_id"};
	std::vector<std::string> v_need_field(need_fields, need_fields + sizeof(need_fields)/sizeof(need_fields[0]));
	ASSERT(CDBMgr::check_fields(res, v_need_field));

	MYSQL_ROW row;
	while (row = mysql_fetch_row(res))
	{
		stBattle *pBattle = new stBattle;
		memset(pBattle, 0, sizeof stBattle);

		int col = 0;
		pBattle->iBattleID = atoi(row[col++]);

		//char* pBuf = UTF8ToANSI(row[col++]);
		//std::vector<std::string> arr;
		//split_string(std::string(pBuf), arr, "|");
		//free(pBuf);

		//int i = 0;
		//for (std::vector<std::string>::iterator it = arr.begin() ; it != arr.end() ; ++it)
		//{
		//	std::vector<int> info;
		//	split_string_2i(*it, info, "-");
		//	ASSERT(info.size() == 4);
		//	pBattle->elements[i].eElementType = (EELEMENTTYPE)info[0];
		//	pBattle->elements[i].iElementSubType = info[1];
		//	pBattle->elements[i].iElementID = info[2];
		//	pBattle->elements[i].iElementPos = info[3];
		//}

		ASSERT(AddBattle(pBattle));
	}
	mysql_free_result(res);
	return true;
}

bool CFightMgr::AddBattle( stBattle* pBattle )
{
	ASSERT(pBattle);
	if (m_mapBattleID2Battle.find(pBattle->iBattleID) != m_mapBattleID2Battle.end())
	{
		printf("error!!有重复的battleID！");
		ASSERT(false);
		return false;
	}

	m_mapBattleID2Battle[pBattle->iBattleID] = pBattle;
	return true;
}