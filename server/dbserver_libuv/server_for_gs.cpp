#include "server_for_gs.h"

#include "str.h"
#include "guid_generator.h"

#include "libuv_helper.h"
#include "net_config.h"

#include "db_mgr.h"
#include "game_config.h"

#include "db_async_command.h"

#include "user_mgr.h"
#include "mail_mgr.h"
bool CServerForGS::Init()
{
	ASSERT(add_message_handle(e_msg_gs_dbs_user_info_req, handle_msg_user_info_req));
	ASSERT(add_message_handle(e_msg_gs_dbs_user_info_op_req, handle_msg_user_info_op_req));
	ASSERT(add_message_handle(e_msg_gs_dbs_user_info_new_create_req, handle_msg_user_info_new_create_req));
	ASSERT(add_message_handle(e_msg_gs_dbs_user_new_mail_req, handle_msg_user_new_mail_req));
	ASSERT(add_message_handle(e_msg_gs_dbs_edit_name_req, handle_msg_edit_name_req));
	ASSERT(add_message_handle(e_msg_gs_dbs_user_new_arena_mail_req, handle_msg_user_new_arena_mail_req));
	ASSERT(add_message_handle(e_msg_gs_dbs_user_friend_op_req, handle_msg_user_friend_op_req));
	ASSERT(add_message_handle(e_msg_gs_dbs_search_user_req, handle_msg_search_user_req));
	ASSERT(add_message_handle(e_msg_gs_dbs_friend_offline_req, handle_msg_friend_offline_req));
	ASSERT(add_message_handle(e_msg_gs_dbs_rank_req, handle_msg_rank_req));
	return true;
}

bool CServerForGS::Start()
{
	int r = uv_thread_create(&m_thread, run, this);
	return r == 0;
}

bool CServerForGS::Join()
{
	int r = uv_thread_join(&m_thread);
	return r == 0;
}

void CServerForGS::run( void* arg )
{
	struct sockaddr_in addr;
	int r;

	CServerForGS* server_for_gs = (CServerForGS*)arg;
	ASSERT(server_for_gs);

	server_for_gs->m_loop = (uv_loop_t*)malloc(sizeof uv_loop_t);
	uv_loop_init(server_for_gs->m_loop);

	const ServerForGSConfig& cfg = GameConfig::GetInstance()->GetServerForGSConfig();
	const char * ip = cfg.m_ip.c_str();
	int port = cfg.m_port;

	r = uv_ip4_addr(ip, port, &addr);
	ASSERT(r == 0);

	r = uv_tcp_init(server_for_gs->m_loop, &server_for_gs->m_server);
	ASSERT(r == 0);

	r = uv_tcp_bind(&server_for_gs->m_server, (const struct sockaddr*) &addr, 0);
	ASSERT(r == 0);

	server_for_gs->m_server.data = server_for_gs;

	r = uv_listen((uv_stream_t*)&server_for_gs->m_server, 128, on_client_connection);
	ASSERT(r == 0);

	printf("ServerForGS start success! listen %s:%d \n", ip, port);
	r = uv_run(server_for_gs->m_loop, UV_RUN_DEFAULT);
	ASSERT(0 && "server exit!");

	uv_loop_close(server_for_gs->m_loop);
	free(server_for_gs->m_loop);
}


void CServerForGS::on_client_connection( uv_stream_t* stream, int status )
{
	int r;
	struct sockaddr sockname, peername;
	int namelen;

	ASSERT(stream);
	ASSERT(status == 0);
	CServerForGS* server_for_gs = (CServerForGS*)stream->data;
	ASSERT(server_for_gs);
	ASSERT(stream == (uv_stream_t*)&(server_for_gs->m_server));
	
	uv_tcp_t* handle = new uv_tcp_t;
	ASSERT(handle);

	r = uv_tcp_init(stream->loop, handle);
	ASSERT(r == 0);

	printf("new connect incoming, before accept! \n");
	r = uv_accept(stream, (uv_stream_t*)handle);
	ASSERT(r == 0);

	memset(&sockname, -1, sizeof sockname);  
	namelen = sizeof sockname;  
	r = uv_tcp_getsockname(handle, &sockname, &namelen);  
	//printf("the r is %d.\n", r);
	ASSERT( r == 0);

	char ip[17] = {0};
	struct sockaddr_in sock_addr = *(struct sockaddr_in*) &sockname; 
	//网络字节序转换成主机字符序  
	uv_ip4_name(&sock_addr, (char*)ip, sizeof ip);  
	printf("getsockname   %s : %d \n", ip, sock_addr.sin_port);


	//有连接，可以获得目标的ip和端口  
	namelen = sizeof peername;  
	r = uv_tcp_getpeername(handle, &peername, &namelen);  
	//printf("the r is %d.\n", r);
	ASSERT( r == 0);

	memset(ip, 0, sizeof ip);
	sock_addr = *(struct sockaddr_in*) &peername; 
	//网络字节序转换成主机字符序  
	uv_ip4_name(&sock_addr, (char*)ip, sizeof ip);  
	printf("getpeername  %s : %d \n", ip, sock_addr.sin_port);

	//todo net_protect作一些限制处理

	printf("new connect accepted! \n");
	r = uv_read_start((uv_stream_t*)handle, alloc_cb, on_receive);
	ASSERT(r == 0);
	
	bool ret = CServerForGS::GetInstance()->add_client(handle);
	ASSERT(ret);

	printf("CServerForGS::add_client 链接进来GS的链接！ip:port  [%s:%d] \n", ip, sock_addr.sin_port);
}

void CServerForGS::on_receive( uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf )
{
	ASSERT(stream);

	int r;
	//todo net_protect作一些限制处理
	if (nread > 0){
		CServerForGS::GetInstance()->process_msg(stream, buf->base, nread);
		free(buf->base);//释放内存 
	}
	else if (nread == 0)//OK, but nothing read
	{

	}
	else{

		//ASSERT(uv_last_error(stream->loop).code == UV_EOF);

		if (nread == UV_EOF) {
			printf("gs链接主动断开\n");

		} else if (nread == UV_ECONNRESET) {
			printf("gs链接异常断开\n");

		} else {
			printf("gs链接异常 %s : %s \n", uv_err_name(nread), uv_strerror(nread));
		}
		uv_shutdown_t* req = (uv_shutdown_t*)req_alloc();
		r = uv_shutdown(req, stream, after_client_shutdown);
		ASSERT(r == 0);
	}
}


bool CServerForGS::send( uv_stream_t* stream, UINT32 msg_id, google::protobuf::Message* pMessage/*, uv_write_cb write_cb*/ )
{
	ASSERT(uv_is_writable(stream));
	uv_buf_t ret_buf;
	UINT32 body_len = pMessage->ByteSize();
	//UINT32 head_len = sizeof(UINT32) + sizeof(UINT32);//4直接的包长度 + 4字节的消息id
	buf_alloc((uv_handle_t*)stream, PACKET_HEAD_LEN + body_len, &ret_buf);

	ASSERT(ret_buf.base && ret_buf.len);
	*((UINT32*)(ret_buf.base)) = PACKET_HEAD_LEN + body_len;
	*((UINT32*)(ret_buf.base)+1) = msg_id;
	INT32 r = pMessage->SerializeToArray(ret_buf.base + PACKET_HEAD_LEN, body_len);
	ASSERT(r);

	uv_write_t* write_req = (uv_write_t*)req_alloc();
	r = uv_write(write_req, stream, &ret_buf, 1, after_send);
	buf_free(&ret_buf);

	ASSERT(r == 0);
	if (r != 0)
	{
		PRINTF_UV_ERROR(r);
		return false;
	}
	else{
		printf("CServerForGS::send msgid[%d]  bytes len[%d] \n", msg_id, PACKET_HEAD_LEN + body_len);
		return true;
	}
}

void CServerForGS::after_send( uv_write_t* req, int status )
{
	ASSERT(req != NULL);
	if (status == 0)
	{
		//write_cb_called++;
		printf("CServerForGS::after_send 给GS发送信息成功\n");
	}
	else{
		if (status == UV_EOF) {
			printf("写时, gs链接主动断开\n");

		} else if (status == UV_ECONNRESET) {
			printf("写时, gs链接异常断开\n");

		} else {
			printf("%s : %s \n", uv_err_name(status), uv_strerror(status));
		}

		uv_shutdown_t* shut_t = (uv_shutdown_t*)req_alloc();
		int r = uv_shutdown(shut_t, req->handle, after_client_shutdown);
		ASSERT(r == 0);
	}

	req_free((uv_req_t*)req);
}

void CServerForGS::after_client_shutdown( uv_shutdown_t* req, int status )
{
	ASSERT(req);
	ASSERT(req->handle);
	uv_close((uv_handle_t*)req->handle, after_client_close);
	req_free((uv_req_t*)req);
}

void CServerForGS::after_client_close( uv_handle_t* handle )
{
	ASSERT(handle);
	CGSConnectContext* ctx = (CGSConnectContext*)handle->data;
	ASSERT(ctx);
	ASSERT(CServerForGS::GetInstance()->remove_client(ctx->ConnectID));
}


bool CServerForGS::process_msg( uv_stream_t* stream, const char * buf, int len )
{
	ASSERT(buf);
	ASSERT(len >= PACKET_HEAD_LEN);//

	char* p = NULL;//遍历buf的工作指针
	int bytes_left = 0;

	if(m_msg_tmp_buf.buf.base){//假如上一个包不完整！

		char* tmp_buf = m_msg_tmp_buf.buf.base;
		UINT32 last_msg_total_len = m_msg_tmp_buf.buf.len;
		UINT32 last_msg_received_len = m_msg_tmp_buf.used_len;

		ASSERT(last_msg_total_len > last_msg_received_len);//不可能等 因为如果相等就已经直接处理了！
		ASSERT(last_msg_total_len >= PACKET_HEAD_LEN);

		UINT32 last_msg_id = *(UINT32*)(tmp_buf + sizeof UINT32);
		ASSERT( last_msg_id < e_msg_gs_dbs::e_msg_gs_dbs_max && last_msg_id > e_msg_gs_dbs::e_msg_gs_dbs_min);

		if( (last_msg_received_len + len) >= last_msg_total_len ){//假如包可以完整了

			memcpy(tmp_buf + last_msg_received_len, buf, last_msg_total_len - last_msg_received_len);

			func_message_handle pMsgHandle = m_mapHandle[last_msg_id];
			if (pMsgHandle)
			{
				bool r = pMsgHandle(stream, tmp_buf + PACKET_HEAD_LEN, last_msg_total_len - PACKET_HEAD_LEN);
			}
			else{
				printf("CServerForGS::process_msg没有找到消息msgid[%d][%s]的处理函数\n", last_msg_id, e_msg_gs_dbs_Name((e_msg_gs_dbs)last_msg_id).c_str());
				//return false;
			}

			//释放空间
			buf_free(&(m_msg_tmp_buf.buf));
			m_msg_tmp_buf.used_len = 0;

			//设置工作指针到未处理区域
			p = const_cast<char*>(buf + (last_msg_total_len - last_msg_received_len ));//工作指针后移到
			bytes_left = len - (last_msg_total_len - last_msg_received_len );
		}
		else{//还是不足一个包的时候
			memcpy(tmp_buf + last_msg_received_len, buf, len);

			m_msg_tmp_buf.used_len = last_msg_received_len + len;

			printf("CServerForGS::process_msg 消息msgid[%d][%s]包不完整 原长[%u]，已接收[%u] 继续拼包\n", last_msg_id, e_msg_gs_dbs_Name((e_msg_gs_dbs)last_msg_id).c_str(), m_msg_tmp_buf.buf.len, m_msg_tmp_buf.used_len);
			return true;
		}
	}
	else{//上一个包是完整处理的
		p = const_cast<char*>(buf);
		bytes_left = len;
	}

	while (bytes_left != 0)
	{
		UINT32 msg_len = *(UINT32 *)(p);

		//ASSERT(bytes_left >= msg_len);
		if(bytes_left >= msg_len){
			bytes_left -= msg_len;

			p += sizeof msg_len;

			UINT32 msg_id = *(UINT32 *)(p);
			ASSERT(msg_id < e_msg_gs_dbs::e_msg_gs_dbs_max && msg_id > e_msg_gs_dbs::e_msg_gs_dbs_min);
			p += sizeof msg_id;

			func_message_handle pMsgHandle = m_mapHandle[msg_id];
			if (pMsgHandle)
			{
				bool r = pMsgHandle(stream, p, msg_len - sizeof(msg_len) - sizeof(msg_id));
			}
			else{
				printf("CServerForGS::process_msg没有找到消息msgid[%d][%s]的处理函数\n", msg_id, e_msg_gs_dbs_Name((e_msg_gs_dbs)msg_id).c_str());
				//return false;
			}

			p += msg_len - sizeof(msg_len) - sizeof(msg_id);
		}
		else{
			ASSERT(msg_len <= 16777216);//不超过16M！
			ASSERT(m_msg_tmp_buf.buf.base == NULL && m_msg_tmp_buf.buf.len == 0 && m_msg_tmp_buf.used_len == 0);
			buf_alloc(NULL, msg_len, &(m_msg_tmp_buf.buf));

			memcpy(m_msg_tmp_buf.buf.base, p, bytes_left);
			m_msg_tmp_buf.used_len = bytes_left;
			printf("CServerForGS::process_msg 消息包不完整 原长[%u]，已接收[%u] 后面的需要拼包...\n", msg_len, bytes_left);
			break;//跳出处理！
		}
	}

	return true;
}

bool CServerForGS::add_message_handle( e_msg_gs_dbs msgID, func_message_handle handle )
{
	if (m_mapHandle[msgID])
	{
		printf("already exsit handle for %u", msgID);
		return false;
	}

	m_mapHandle[msgID] = handle;
	return true;
}

uv_async_t* CServerForGS::CreateAsync()
{
	ASSERT(m_loop);
	uv_async_t* async = (uv_async_t*)malloc(sizeof uv_async_t);
	ASSERT(async);
	int r = uv_async_init(m_loop, async, async_cb);
	ASSERT(r == 0);
	return async;
}

void CServerForGS::DestroyAsync( uv_async_t* handle )
{
	ASSERT(handle);
	free(handle);
}

void CServerForGS::async_cb( uv_async_t* handle )
{
	ASSERT(handle);
	stGSAsyncData* asyncData = (stGSAsyncData*)handle->data;
	ASSERT(asyncData);
	//do something
	switch (asyncData->op)
	{
	case stGSAsyncData::EGSAsyncOP_None:
		{
			printf("CServerForGS::async_cb  EGSAsyncOP_None! \n");
		}break;
	case stGSAsyncData::EGSAsyncOP_User_Info_Res:
		{
			uv_tcp_t* client = CServerForGS::GetInstance()->get_client(asyncData->gs_connect_id);
			//db_user_info* user_info = (db_user_info*)asyncData->data;
			stDBGetUserInfoRet* db_ret = (stDBGetUserInfoRet*)asyncData->data;
			ASSERT(db_ret);
			db_user_info* user_info = db_ret->user_info;

			if (!client)
			{
				//delete user_info;
				delete db_ret->user_info; 
				db_ret->user_info = NULL;
				delete db_ret;
				printf("\n EGSAsyncOP_Get_User_Info 回调回来 dbs上的gs链接已经断开！！！\n ");
			}
			else
			{
				dbs_gs_user_info_res res;
				res.set_user_id(asyncData->user_id);
				BYTE connect_id_buf[GUID_BYTE_LEN] = {0};
				asyncData->user_connect_id.ToBytes(connect_id_buf, GUID_BYTE_LEN);
				res.set_user_connect_id(connect_id_buf, GUID_BYTE_LEN);
				res.set_allocated_user_info(user_info);//设置为null的时候就会清理内部对象 并且设置为未设置状态！
				//if (user_info)
				if (db_ret->ret == stDBGetUserInfoRet::Success)//读取玩家信息OK
				{
					ASSERT(user_info);
					CMailMgr::GetInstance()->UnReceivedToReceived(db_ret->user_info);
					//内存中保存一份用户信息 以便更快返回！
					//之前并不存在此用户信息！！ 假如存在就直接返回给gs 而不需要去查数据库
					ASSERT(CUserMgr::GetInstance()->GetUserInfo(asyncData->user_id) == NULL);//这个地方可能会触发断言 如果重复的快速登录同一个账户的时候！ 两次登陆都判断没有用户信息 然后两次查询 查询好了之后就有了两份用户信息！
					//ASSERT(CUserMgr::GetInstance()->Add(asyncData->user_id, user_info));
					if (CUserMgr::GetInstance()->GetUserInfo(asyncData->user_id) == NULL)
					{
						bool r = CUserMgr::GetInstance()->Add(asyncData->user_id, user_info);
						ASSERT(r);
					}
					
					res.set_error_code(e_dbsgs_errorcode_success);
				}
				else if(db_ret->ret == stDBGetUserInfoRet::Not_Exist_User){//玩家信息不存在 此时在GS上创建 并告知DBS
					ASSERT(user_info == NULL);
					
					res.set_error_code(e_dbsgs_errorcode_user_info_not_exsit);
				}
				else{
					res.set_error_code(e_dbsgs_errorcode_unkown_error);
				}
				
				bool r = send((uv_stream_t*)client, e_msg_dbs_gs_user_info_res, &res);
				//res.set_allocated_user_info(NULL);//NO！！这种会释放掉前面保存user_info
				res.release_user_info();//用这种 否则析构的时候会释放掉内部的user_info
				ASSERT(r == true);

				//不要释放user_info  如果存在就需要保存在usermgr中！不存在或者出错这里就是NULL
			}

			delete db_ret;

		}break;
	case stGSAsyncData::EGSAsyncOP_User_Edit_User_Name_Res:{
			uv_tcp_t* client = CServerForGS::GetInstance()->get_client(asyncData->gs_connect_id);
			stDBEditUserNameRet* db_ret = (stDBEditUserNameRet*)asyncData->data;
			ASSERT(db_ret);
			if (!client)
			{
				printf("\n EGSAsyncOP_User_Edit_User_Name_Res 回调回来 dbs上的gs链接已经断开！！！\n ");
			}
			else
			{
				dbs_gs_edit_name_res res;
				res.set_user_id(asyncData->user_id);
				if (stDBEditUserNameRet::Success == db_ret->ret)
				{
					res.set_error_code(e_dbsgs_errorcode_success);
					res.set_name(db_ret->name);
					//需要同步玩家姓名到dbs
					gs_dbs_user_info_op_req db_req;
					db_req.set_user_id(asyncData->user_id);
					db_req.mutable_base_info()->set_name(db_ret->name);
					db_req.mutable_base_info()->set_op_type(gs_dbs_user_info_op_req::Update);
					CUserMgr::GetInstance()->Update(&db_req);
				}
				else if(stDBEditUserNameRet::Same_Name == db_ret->ret)
				{
					res.set_error_code(e_dbsgs_errorcode_user_edit_same_name);
				}
				else
				{
					res.set_error_code(e_dbsgs_errorcode_unkown_error);
				}
				bool r = send((uv_stream_t*)client, e_msg_dbs_gs_edit_name_res, &res);
			}
			delete db_ret;
			db_ret = NULL;

		}break;
	case stGSAsyncData::EGSAsyncOP_User_Search_User:{
		uv_tcp_t* client = CServerForGS::GetInstance()->get_client(asyncData->gs_connect_id);
		dbs_gs_search_user_res* db_ret = (dbs_gs_search_user_res*)asyncData->data;
		ASSERT(db_ret);
		if (!client)
		{
			printf("\n EGSAsyncOP_User_Search_User 回调回来 dbs上的gs链接已经断开！！！\n ");
		}
		else
		{
			db_ret->set_user_id(asyncData->user_id);//直接发送db_ret
			bool r = send((uv_stream_t*)client, e_msg_dbs_gs_search_user_res, db_ret);
		}
		delete db_ret;
		db_ret = NULL;

		}break;
	case stGSAsyncData::EGSAsyncOP_User_Friend_OffLine_Res:{
		uv_tcp_t* client = CServerForGS::GetInstance()->get_client(asyncData->gs_connect_id);
		stDBFriendOffLineRet* db_ret = (stDBFriendOffLineRet*)asyncData->data;
		ASSERT(db_ret);
		if (!client)
		{
			printf("\n EGSAsyncOP_User_Friend_OffLine_Res 回调回来 dbs上的gs链接已经断开！！！\n ");
		}
		else{
			dbs_gs_friend_offline_res res;
			res.set_op_type(db_ret->req.op_type());
			res.set_user_id(db_ret->req.user_id());
			switch(db_ret->req.op_type()){
			case gs_dbs_friend_offline_req::ADD_FRIEND:{
				if(db_ret->ret == stDBFriendOffLineRet::SUCCESS)
				{
					//更新db内存
					res.set_error_code(e_dbsgs_errorcode_success);

					//需要同步玩家信息到dbs(两边都要同步)
					//me
					gs_dbs_friend_offline_req& req = db_ret->req;
					UINT32 iUserID = req.user_id();
					if(CUserMgr::GetInstance()->GetUserInfo(iUserID)){//在线直接同步(应该是在线的)
						gs_dbs_user_info_op_req db_req; 
						db_req.set_user_id(iUserID);

						gs_dbs_user_info_op_req::struct_op_friend* op_friend = db_req.add_op_friends();
						op_friend->set_op_type(gs_dbs_user_info_op_req::struct_op_friend::ADD_FRIEND_GS_ONLINE);
						op_friend->mutable_base_info()->CopyFrom(req.friend_info());
						CUserMgr::GetInstance()->Update(&db_req);


					}
					//friend(加到其待确认列表中)
					gs_dbs_user_friend_op_req friend_req;
					friend_req.set_op_type(gs_dbs_user_friend_op_req::FRIEND_ADD_FRIEND);
					friend_req.set_user_id(req.friend_info().user_id());
					friend_req.mutable_base_info()->CopyFrom(req.me_info());
					//处理
					CUserMgr::GetInstance()->UpdateFriend(friend_req);

					res.mutable_info()->CopyFrom(req.friend_info());
					send((uv_stream_t*)client, e_msg_dbs_gs_friend_offline_res, &res);


				}
				else if(db_ret->ret == stDBFriendOffLineRet::FRIEND_CONFIRM_FULL){//直接发送客户端

					res.set_error_code(e_dbsgs_errorcode_friend_add_confirm_reach_max);
					send((uv_stream_t*)client, e_msg_dbs_gs_friend_offline_res, &res);
				}
				else if(db_ret->ret == stDBFriendOffLineRet::Unkown_Error){
					res.set_error_code(e_dbsgs_errorcode_unkown_error);
					send((uv_stream_t*)client, e_msg_dbs_gs_friend_offline_res, &res);
				}
													   }break;
			case gs_dbs_friend_offline_req::AFFIRM_AGREEN:
			case gs_dbs_friend_offline_req::AFFIRM_AGREE1:{
				
				if (db_ret->ret == stDBFriendOffLineRet::SUCCESS)
				{
					//同步信息(两边都要同步)
					res.set_error_code(e_dbsgs_errorcode_success);
					
					//me(从待确认列表移到好友列表)
					gs_dbs_friend_offline_req& req = db_ret->req;
					UINT32 iUserID = req.user_id();
					if(CUserMgr::GetInstance()->GetUserInfo(iUserID)){//在线直接同步(应该是在线的)
						gs_dbs_user_info_op_req db_req; 
						db_req.set_user_id(iUserID);

						gs_dbs_user_info_op_req::struct_op_friend* op_friend = db_req.add_op_friends();
						if (gs_dbs_friend_offline_req::AFFIRM_AGREE1 == req.op_type())
						{
							op_friend->set_friend_id(req.affirm_ids(0));
							op_friend->set_op_type(gs_dbs_user_info_op_req::struct_op_friend::AFFIRM_AGREE1);
						}
						else if(gs_dbs_friend_offline_req::AFFIRM_AGREEN == req.op_type()){
							op_friend->set_op_type(gs_dbs_user_info_op_req::struct_op_friend::AFFIRM_AGREEN);
						}
						CUserMgr::GetInstance()->Update(&db_req);
					}
					//friend(从申请列表移动到好友列表) 
					gs_dbs_user_friend_op_req friend_req;
					friend_req.set_op_type(gs_dbs_user_friend_op_req::FRIEND_AFFIRM_AGREE);
					::google::protobuf::RepeatedField< ::google::protobuf::uint32 >::iterator it = req.mutable_affirm_ids()->begin();
					for (; it != req.mutable_affirm_ids()->end(); ++it)
					{
						friend_req.set_user_id(*it);
						friend_req.set_friend_user_id(iUserID);
						//处理
						CUserMgr::GetInstance()->UpdateFriend(friend_req);
					}
					res.mutable_affirm_ids()->CopyFrom(req.affirm_ids());
				}
				else if(db_ret->ret == stDBFriendOffLineRet::FRIEND_CONFIRM_FULL)
				{
					res.set_error_code(e_dbsgs_errorcode_friend_num_reach_max);
				}
				else if (db_ret->ret == stDBFriendOffLineRet::System_Error || db_ret->ret == stDBFriendOffLineRet::Unkown_Error)
				{
					res.set_error_code(e_dbsgs_errorcode_unkown_error);
				}
				send((uv_stream_t*)client, e_msg_dbs_gs_friend_offline_res, &res);
														 }break;
			}

			
		}
		delete db_ret;
		db_ret = NULL;
		}break;
	case stGSAsyncData::EGSAsyncOP_Rank_res:{
		uv_tcp_t* client = CServerForGS::GetInstance()->get_client(asyncData->gs_connect_id);
		dbs_gs_rank_res* db_ret = (dbs_gs_rank_res*)asyncData->data;
		ASSERT(db_ret);
		if (!client)
		{
			printf("\n EGSAsyncOP_Rank_res 回调回来 dbs上的gs链接已经断开！！！\n ");
		}
		else
		{
			bool r = send((uv_stream_t*)client, e_msg_dbs_gs_rank_res, db_ret);
		}
		delete db_ret;
		db_ret = NULL;
	}break;
	}

	delete asyncData;
	uv_close((uv_handle_t*)handle, after_async_close);
}

void CServerForGS::after_async_close( uv_handle_t* handle )
{
	ASSERT(handle != NULL);
	//free((uv_async_t*)handle);
	CServerForGS::GetInstance()->DestroyAsync((uv_async_t*)handle);
}

CGSConnectContext* CServerForGS::attach_context_on_client(uv_tcp_t* client)
{
	CGSConnectContext* ctx = new CGSConnectContext;
	ASSERT(ctx);
	client->data = ctx;
	return ctx;
}

bool CServerForGS::detach_context_on_client(uv_tcp_t* client)
{
	ASSERT(client);
	CGSConnectContext* ctx = (CGSConnectContext*)client->data;
	ASSERT(ctx);

	delete ctx;
	client->data = NULL;

	return true;

}

bool CServerForGS::add_client(uv_tcp_t* client)
{
	ASSERT(client);

	Guid connect_id = GetGuidGenerator().newGuid();
	ASSERT(m_clients.find(connect_id) == m_clients.end());

	CGSConnectContext* ctx = attach_context_on_client(client);
	ASSERT(ctx);
	ctx->ConnectID = connect_id;

	m_clients[connect_id] = client;
	return true;
}

bool CServerForGS::remove_client(const Guid& connect_id)
{
	std::map<Guid, uv_tcp_t*>::iterator it = m_clients.find(connect_id);
	if (it != m_clients.end())
	{
		ASSERT(it->second);
		
		detach_context_on_client(it->second);

		delete it->second;
		m_clients.erase(it);
		return true;
	}
	else
	{
		return false;
	}
}

uv_tcp_t* CServerForGS::get_client(const Guid& connect_id)
{
	std::map<Guid, uv_tcp_t*>::iterator it = m_clients.find(connect_id);
	if (it != m_clients.end())
	{
		return it->second;
	}
	else
	{
		return NULL;
	}
}


bool CServerForGS::handle_msg_user_info_req(uv_stream_t* stream, const char * buf, int len)
{
	//gs_dbs_userinfo_req req;
	//bool r = req.ParseFromArray(buf, len);
	//
	//if (r)
	//{
	//	int id = req.id();
	//	//test(直接发送给gs)
	//	dbs_gs_userinfo_res res;
	//	res.set_error_code(e_dbsgs_errorcode_success);
	//	send(stream, e_msg_dbs_gs_userinfo_res, &res);
	//	return true;
	//}
	//else{
	//	PRINTF_PBPARSE_ERROR(e_msg_gs_dbs_userinfo_req);
	//	return false;
	//}	

	ASSERT(stream);

	gs_dbs_user_info_req req;
	bool r = req.ParseFromArray(buf, len);

	if (r)
	{
		//int id = req.id();
		////test(直接发送给gs)
		//dbs_gs_user_info_res res;
		//res.set_error_code(e_dbsgs_errorcode_success);
		//send(stream, e_msg_dbs_gs_user_info_res, &res);

		db_user_info* user_info = CUserMgr::GetInstance()->GetUserInfo(req.user_id());
		if (user_info)
		{
			printf("DBS上已有玩家[%u]信息，更快返回玩家信息请求！\n", req.user_id());
			dbs_gs_user_info_res res;
			res.set_error_code(e_dbsgs_errorcode_success);
			res.set_user_id(user_info->user_base_info().user_id());
			res.set_user_connect_id(*(req.mutable_user_connect_id()));
			res.set_allocated_user_info(user_info);//设置为null的时候就会清理内部对象 并且设置为未设置状态！
			bool r = send(stream, e_msg_dbs_gs_user_info_res, &res);
			res.release_user_info();//！！！一定要作此操作！ 否则就把db上的用户信息给释放了！
			ASSERT(r);
			return true;
		}

		printf("DBS上没有玩家[%u]信息，开始数据库查询！\n", req.user_id());
		CGSConnectContext* ctx = (CGSConnectContext*)stream->data;
		ASSERT(ctx);

		CUserCommand* pCommand = new CUserCommand;
		pCommand->Optype(CUserCommand::GET_INFO);
		pCommand->Gs_connect_id(ctx->ConnectID);
		pCommand->User_connect_id(Guid((BYTE*)(req.mutable_user_connect_id()->c_str())));
		pCommand->User_id(req.user_id());

		CAsyncCommandModule::GetInstance()->AddCommand(pCommand);

		return true;
	}
	else{
		PRINTF_PBPARSE_ERROR(e_msg_gs_dbs_user_info_req);
		dbs_gs_user_info_res res;
		res.set_error_code(e_dbsgs_errorcode_unkown_error);
		bool r = send(stream, e_msg_dbs_gs_user_info_res, &res);
		ASSERT(r);
		return false;
	}	
}

bool CServerForGS::handle_msg_user_info_op_req( uv_stream_t* stream, const char * buf, int len )
{
	ASSERT(stream);

	gs_dbs_user_info_op_req* req = new gs_dbs_user_info_op_req();
	bool r = req->ParseFromArray(buf, len);
	if (r)
	{
		//int id = req.id();
		////test(直接发送给gs)
		//dbs_gs_user_info_res res;
		//res.set_error_code(e_dbsgs_errorcode_success);
		//send(stream, e_msg_dbs_gs_user_info_res, &res);
		printf("dbs处理gs的玩家信息修改请求 handle_msg_user_info_op_req \n");
		//req->PrintDebugString();

		if(req->has_new_mail()){
			dbs_gs_user_new_mail_res res;
			CMailMgr::GetInstance()->HandleDBOPNewMailReq(*req, res);
			send(stream, e_msg_dbs_gs_user_new_mail_res, &res);
		}
		
		//1. 更新内存
		CUserMgr::GetInstance()->Update(req);
		
		//2. 更新数据库
		CGSConnectContext* ctx = (CGSConnectContext*)stream->data;
		ASSERT(ctx);

		CUserCommand* pCommand = new CUserCommand;
		pCommand->Optype(CUserCommand::OP_INFO);
		pCommand->Gs_connect_id(ctx->ConnectID);
		pCommand->User_id(req->user_id());
		pCommand->Data(req);//保持对象

		CAsyncCommandModule::GetInstance()->AddCommand(pCommand);

		return true;
	}
	else{
		delete req;
		PRINTF_PBPARSE_ERROR(e_msg_gs_dbs_user_info_op_req);
		dbs_gs_user_info_op_res res;
		res.set_error_code(e_dbsgs_errorcode_unkown_error);

		bool r = send(stream, e_msg_dbs_gs_user_info_op_res, &res);
		ASSERT(r);
		ASSERT(false);//不应该失败！ GS是我们控制的 不至于包都解析失败！
		return false;
	}	
}

bool CServerForGS::handle_msg_user_info_new_create_req( uv_stream_t* stream, const char * buf, int len )
{
	ASSERT(stream);
	gs_dbs_user_info_new_create_req* req = new gs_dbs_user_info_new_create_req();
	dbs_gs_user_info_new_create_res res;
	bool r = req->ParseFromArray(buf, len);
	if (r)
	{
		ASSERT(req->has_user_info());
		printf("准备创建新用户数据[%u]到DB \n", req->user_info().user_base_info().user_id());
		//1. 添加进内存
		db_user_info* user_info = req->release_user_info();
		r = CUserMgr::GetInstance()->Add(user_info->user_base_info().user_id(), user_info);
		ASSERT(r);//todo !假如失败了 则要删除 db_user_info!
		if (!r)
		{
			res.set_error_code(e_dbsgs_errorcode_unkown_error);
			send(stream, e_msg_dbs_gs_user_info_new_create_res, &res);

			delete user_info;
			delete req;
			return false;
		}
		else{
			//2. 添加进数据库
			CGSConnectContext* ctx = (CGSConnectContext*)stream->data;
			ASSERT(ctx);

			CUserCommand* pCommand = new CUserCommand;
			pCommand->Optype(CUserCommand::New_Create);
			pCommand->Gs_connect_id(ctx->ConnectID);
			pCommand->User_id(req->user_info().user_base_info().user_id());
			pCommand->Data(user_info);//保持对象

			CAsyncCommandModule::GetInstance()->AddCommand(pCommand);

			delete req;
			return true;
		}
	}
	else{
		PRINTF_PBPARSE_ERROR(e_msg_gs_dbs_user_info_new_create_req);

		res.set_error_code(e_dbsgs_errorcode_unkown_error);
		send(stream, e_msg_dbs_gs_user_info_new_create_res, &res);

		delete req;
		return false;
	}

	return false;
}

bool CServerForGS::handle_msg_edit_name_req(uv_stream_t* stream, const char * buf, int len){
	ASSERT(stream);
	gs_dbs_edit_name_req* req = new gs_dbs_edit_name_req;
	bool r = req->ParseFromArray(buf, len);
	if (r)
	{
		CGSConnectContext* ctx = (CGSConnectContext*)stream->data;
		ASSERT(ctx);

		CUserCommand* pCommand = new CUserCommand;
		pCommand->Optype(CUserCommand::OP_NAME);
		pCommand->Gs_connect_id(ctx->ConnectID);
		//pCommand->User_connect_id(Guid((BYTE*)(req.mutable_user_connect_id()->c_str())));
		pCommand->User_id(req->user_id());
		pCommand->Data(req);
		CAsyncCommandModule::GetInstance()->AddCommand(pCommand);

		return true;
	}
	else{
		PRINTF_PBPARSE_ERROR(e_msg_gs_dbs_edit_name_req);
		dbs_gs_edit_name_res res;
		res.set_error_code(e_dbsgs_errorcode_unkown_error);
		send(stream, e_msg_dbs_gs_edit_name_res, &res);
		delete req;
		return false;
	}
	return true;
}

bool CServerForGS::handle_msg_user_new_mail_req(uv_stream_t* stream, const char * buf, int len)
{
	ASSERT(stream);
	gs_dbs_user_new_mail_req req;
	bool r = req.ParseFromArray(buf, len);
	if(r)
	{
		dbs_gs_user_new_mail_res res;
		CMailMgr::GetInstance()->HandleNewMailReq(req, res);
		send(stream, e_msg_dbs_gs_user_new_mail_res, &res);
	}
	else{
		PRINTF_PBPARSE_ERROR(e_msg_gs_dbs_user_new_mail_req);
		dbs_gs_user_new_mail_res res;
		res.set_error_code(e_dbsgs_errorcode_unkown_error);
	}
	return true;
}

bool CServerForGS::handle_msg_user_new_arena_mail_req( uv_stream_t* stream, const char * buf, int len )
{
	ASSERT(stream);
	gs_dbs_user_new_arena_mail_req* req = new gs_dbs_user_new_arena_mail_req;
	bool r = req->ParseFromArray(buf, len);
	if (r)
	{
		dbs_gs_user_new_arena_mail_res res;
		CMailMgr::GetInstance()->HandleNewArenaMailReq(*req, res);
		send(stream, e_msg_dbs_gs_user_new_arena_mail_res, &res);

	}
	else{
		PRINTF_PBPARSE_ERROR(e_msg_gs_dbs_user_new_arena_mail_req);
		dbs_gs_user_new_arena_mail_res res;
		res.set_error_code(e_dbsgs_errorcode_unkown_error);
		delete req;
	}
	return true;
}

bool CServerForGS::handle_msg_user_friend_op_req( uv_stream_t* stream, const char * buf, int len )
{
	ASSERT(stream);
	gs_dbs_user_friend_op_req* req = new gs_dbs_user_friend_op_req;
	bool r = req->ParseFromArray(buf, len);
	if (r)
	{
		CGSConnectContext* ctx = (CGSConnectContext*)stream->data;
		ASSERT(ctx);

		CUserMgr::GetInstance()->UpdateFriend(*req);

		CUserCommand* pCommand = new CUserCommand;
		pCommand->Optype(CUserCommand::OP_FRIEND);
		pCommand->Gs_connect_id(ctx->ConnectID);
		pCommand->User_id(req->user_id());
		pCommand->Data(req);
		CAsyncCommandModule::GetInstance()->AddCommand(pCommand);
	}else{
		PRINTF_PBPARSE_ERROR(e_msg_gs_dbs_user_friend_op_req);
		dbs_gs_user_friend_op_res res;
		res.set_error_code(e_dbsgs_errorcode_unkown_error);
		delete req;
		return false;
	}
	return true;
}

bool CServerForGS::handle_msg_search_user_req( uv_stream_t* stream, const char * buf, int len )
{
	ASSERT(stream);
	gs_dbs_search_user_req* req = new gs_dbs_search_user_req;
	bool r = req->ParseFromArray(buf, len);
	if (r)
	{
		CGSConnectContext* ctx = (CGSConnectContext*)stream->data;
		ASSERT(ctx);

		CUserCommand* pCommand = new CUserCommand;
		pCommand->Optype(CUserCommand::SEARCH_USER); 
		pCommand->Gs_connect_id(ctx->ConnectID);
		//pCommand->User_connect_id(Guid((BYTE*)(req.mutable_user_connect_id()->c_str())));
		pCommand->User_id(req->user_id());
		pCommand->Data(req);
		CAsyncCommandModule::GetInstance()->AddCommand(pCommand);
	}
	else{
		PRINTF_PBPARSE_ERROR(e_msg_gs_dbs_search_user_req);
		dbs_gs_search_user_res res;
		res.set_error_code(e_dbsgs_errorcode_unkown_error);
		delete req;
	}
	return true;
}

bool CServerForGS::handle_msg_friend_offline_req( uv_stream_t* stream, const char * buf, int len )
{
	ASSERT(stream);
	gs_dbs_friend_offline_req* req = new gs_dbs_friend_offline_req;
	bool r = req->ParseFromArray(buf, len);
	if (r) 
	{
		CGSConnectContext* ctx = (CGSConnectContext*)stream->data;
		ASSERT(ctx);
		
		CUserCommand* pCommand = new CUserCommand;
		pCommand->Optype(CUserCommand::OP_FRIEND_OFFLINE);
		pCommand->Gs_connect_id(ctx->ConnectID);
		pCommand->User_id(req->user_id());
		pCommand->Data(req);
		CAsyncCommandModule::GetInstance()->AddCommand(pCommand);
	}
	else{
		PRINTF_PBPARSE_ERROR(e_msg_gs_dbs_friend_offline_req);
		dbs_gs_friend_offline_res res;
		res.set_error_code(e_dbsgs_errorcode_unkown_error);
		delete req;
		return false;
	}
	return true;
}

bool CServerForGS::handle_msg_rank_req(uv_stream_t* stream, const char * buf, int len)
{
	ASSERT(stream);
	gs_dbs_rank_req* req = new gs_dbs_rank_req;
	bool r = req->ParseFromArray(buf, len);
	if (r)
	{
		CGSConnectContext* ctx = (CGSConnectContext*)stream->data;
		ASSERT(ctx);

		CUserCommand* pCommand = new CUserCommand;
		pCommand->Optype(CUserCommand::OP_RANK);
		pCommand->Gs_connect_id(ctx->ConnectID);
		CAsyncCommandModule::GetInstance()->AddCommand(pCommand);
	}
	else{
		PRINTF_PBPARSE_ERROR(e_msg_gs_dbs_rank_req);
		dbs_gs_rank_res res;
		res.set_error_code(e_dbsgs_errorcode_unkown_error);
		delete req;
		return false;
	}
	return true;
}