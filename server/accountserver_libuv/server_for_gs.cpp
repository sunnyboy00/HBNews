#include "server_for_gs.h"

#include "area.h"
#include "str.h"
#include "db_sync_operator.h"

#include "libuv_helper.h"
#include "net_config.h"

#include "db_mgr.h"
#include "server_for_user.h"
#include "game_config.h"
bool CServerForGS::Init()
{
	MYSQL* con = CDBMgr::GetInstance()->GetConnection();
	char sql[1024] = {0};

	sprintf_s(sql, sizeof(sql), "SELECT `area_id`, `name`, `ip`, `port`, `hidden` FROM `area`");

	int code = mysql_real_query(con, sql, (unsigned int)strlen(sql));
	if (0 != code)   //非0查询失败  
	{
		printf("query failed! [%s] [%d] [%s] \n", sql, mysql_errno(con), mysql_error(con));
		return false;  
	}

	MYSQL_RES* res = mysql_store_result(con);   //保存查询结果  
	std::string need_fields[] = {"area_id", "name", "ip", "port", "hidden"};
	std::vector<std::string> v_need_field(need_fields, need_fields + sizeof(need_fields)/sizeof(need_fields[0]));
	ASSERT(CDBMgr::check_fields(res, v_need_field));

	MYSQL_ROW row;
	while (row = mysql_fetch_row(res))
	{
		int col = 0;
		CArea* pArea = new CArea;

		pArea->area_id = atoi(row[col++]);

		ASSERT(m_mapAreaID2Area.find(pArea->area_id) == m_mapAreaID2Area.end());

		char* pBuf = UTF8ToANSI(row[col++]);
		pArea->name = pBuf;
		free(pBuf);

		pBuf = UTF8ToANSI(row[col++]);
		pArea->ip = pBuf;
		free(pBuf);

		pArea->port = atoi(row[col++]);

		pArea->isHidden = (bool)atoi(row[col++]);

		pArea->status = area::EAreaStatus::area_EAreaStatus_EAreaStatus_Good;//设置默认的状态！

		m_mapAreaID2Area[pArea->area_id] = pArea;
		m_mapIP2Area[pArea->ip] = pArea;
	}

	if (m_mapAreaID2Area.size() == 0)
	{
		printf("\n 初始化区服列表失败 没有区服信息!!!");
		return false;
	}

	mysql_free_result(res);
	

	//ASSERT(add_message_handle(e_g_as_msg::e_msg_c_as_register_req, handle_msg_register));
	//ASSERT(add_message_handle(e_msg_c_as::e_msg_c_as_login_req, handle_msg_login));
	//ASSERT(add_message_handle(e_msg_c_as::e_msg_c_as_logout_req, handle_msg_logout));

	ASSERT(add_message_handle(e_msg_gs_as::e_msg_gs_as_enter_area_res, handle_msg_enter_area_res));
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

bool CServerForGS::GetAreaList( area_list* areas )
{
	ASSERT(areas);
	for (std::map<UINT32, CArea*>::iterator it = m_mapAreaID2Area.begin() ; it != m_mapAreaID2Area.end() ; ++it)
	{
		CArea*pArea = it->second;
		ASSERT(pArea);
		area* ele = areas->add_areas();
		ele->set_area_id(pArea->area_id);

		char* pbuf = ANSIToUTF8(pArea->name.c_str());
		ele->set_name(pbuf);
		free(pbuf);

		ele->set_status(pArea->status);
	}

	return true;
}

Guid CServerForGS::add_client( uv_tcp_t* client )
{
	ASSERT(client);

	struct sockaddr peername;
	//有连接，可以获得目标的ip和端口  
	int namelen = sizeof peername;  
	int r = uv_tcp_getpeername(client, &peername, &namelen);  
	//printf("the r is %d.\n", r);
	ASSERT( r == 0);

	char ip[17] = {0};
	memset(ip, 0, sizeof ip);
	struct sockaddr_in sock_addr = *(struct sockaddr_in*) &peername; 
	//网络字节序转换成主机字符序  
	uv_ip4_name(&sock_addr, (char*)ip, sizeof ip);  
	printf("getpeername  %s : %d \n", ip, sock_addr.sin_port);

	std::map<std::string, CArea*>::iterator it = m_mapIP2Area.find(ip); 
	if ( it == m_mapIP2Area.end())
	{
		printf("CServerForGS::add_client 链接进来了非配置GS的链接！ip:port  [%s:%d] 后面需要做功能 直接拒绝他们的链接！\n", ip, sock_addr.sin_port);
		return Guid();
	}

	CArea* pArea = it->second;
	ASSERT(pArea);

	Guid connect_id = GetGuidGenerator().newGuid();
	ASSERT(m_clients.find(connect_id) == m_clients.end());
	m_clients[connect_id] = client;

	pArea->client_id = connect_id;

	CGSContext* ctx = new CGSContext;
	ctx->client_id = connect_id;
	ctx->area_id = pArea->area_id;
	ctx->stauts = area::EAreaStatus::area_EAreaStatus_EAreaStatus_None;
	client->data = ctx;

	printf("CServerForGS::add_client 链接进来GS的链接！ip:port  [%s:%d]  areaid[%d] areaname[%s] \n", ip, sock_addr.sin_port, pArea->area_id, pArea->name.c_str());
	return connect_id;
}

bool CServerForGS::remove_client( Guid key )
{
	
	std::map<Guid, uv_tcp_t*>::iterator it =  m_clients.find(key);
	if (it != m_clients.end())
	{
		ASSERT(it->second);
		CGSContext* ctx = (CGSContext*)it->second->data;
		ASSERT(ctx);
		//ctx->stauts = area::EAreaStatus::area_EAreaStatus_EAreaStatus_None;

		std::map<UINT32, CArea*>::iterator itArea = m_mapAreaID2Area.find(ctx->area_id);
		ASSERT(itArea!= m_mapAreaID2Area.end() && itArea->second);

		itArea->second->client_id = Guid();

		delete ctx;//删除client相关信息
		it->second->data = NULL;
		delete it->second;//删除client
		m_clients.erase(it);
		return true;
	}
	else{
		return false;
	}
}

uv_tcp_t* CServerForGS::get_client( Guid& key )
{
	std::map<Guid, uv_tcp_t*>::iterator it =  m_clients.find(key);
	if (it != m_clients.end())
	{
		return it->second;
	}
	else{
		return NULL;
	}
}

uv_tcp_t* CServerForGS::get_client( UINT32 area_id )
{
	std::map<UINT32, CArea*>::iterator it = m_mapAreaID2Area.find(area_id);
	if (it != m_mapAreaID2Area.end())
	{
		CArea* pArea = it->second;
		ASSERT(pArea);
		return get_client(pArea->client_id);
	}
	else{
		return NULL;
	}
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

	for (std::map<UINT32, CArea*>::iterator it = m_mapAreaID2Area.begin(); it != m_mapAreaID2Area.end();++it)
	{
		delete it->second;
	}

	m_mapAreaID2Area.clear();
	m_mapIP2Area.clear();
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

	CServerForGS::GetInstance()->add_client(handle);
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

		
		CGSContext* ctx = (CGSContext*)stream->data;
		ASSERT(ctx);

		if (nread == UV_EOF) {
			printf("areaid[%d] gs链接主动断开\n", ctx->area_id);

		} else if (nread == UV_ECONNRESET) {
			printf("areaid[%d] gs链接异常断开\n", ctx->area_id);

		} else {
			//printf("areaid[%d] gs链接异常 %s : %s \n", ctx->area_id, uv_err_name(nread), uv_strerror(nread));
			printf("areaid[%d] gs链接异常 \n", ctx->area_id);
			PRINTF_UV_ERROR(nread);
		}
		uv_shutdown_t* req = (uv_shutdown_t*)req_alloc();
		r = uv_shutdown(req, stream, after_client_shutdown);
		ASSERT(r == 0);
	}
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
	//uv_tcp_t* client = (uv_tcp_t*)handle;
	//ASSERT(client);

	ASSERT(handle);
	CGSContext* ctx = (CGSContext*)handle->data;
	ASSERT(ctx);
	//delete client;
	ASSERT(CServerForGS::GetInstance()->remove_client(ctx->client_id));
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
			//printf("%s : %s \n", uv_err_name(status), uv_strerror(status));
			printf("写时, gs链接异常\n");
			PRINTF_UV_ERROR(status);
		}

		uv_shutdown_t* shut_r = (uv_shutdown_t*)req_alloc();
		int r = uv_shutdown(shut_r, req->handle, after_client_shutdown);
		ASSERT(r == 0);
	}

	req_free((uv_req_t*)req);
}

bool CServerForGS::process_msg( uv_stream_t* stream, const char * buf, int len )
{
	ASSERT(buf);
	ASSERT(len >= PACKET_HEAD_LEN);//
	int pos = 0;
	int bytes_left = len;

	char* p = const_cast<char*>(buf);
	while (bytes_left != 0)
	{
		UINT32 msg_len = *(UINT32 *)(p);
		p += sizeof msg_len;

		ASSERT(bytes_left >= msg_len);
		bytes_left -= msg_len;

		UINT32 msg_id = *(UINT32 *)(p);
		ASSERT(msg_id > e_msg_gs_as::e_msg_gs_as_min && msg_id < e_msg_gs_as::e_msg_gs_as_max);
		p += sizeof msg_id;

		func_message_handle pMsgHandle = m_mapHandle[msg_id];
		if (pMsgHandle)
		{
			bool r = pMsgHandle(stream, p, msg_len - sizeof(msg_len) - sizeof(msg_id));
		}
		else{
			printf("CServerForGS::process_msg没有找到msdid[%d][%s]的消息处理函数\n", msg_id, e_msg_gs_as_Name(e_msg_gs_as(msg_id)).c_str());
			//return false;
		}

		p += msg_len - sizeof(msg_len) - sizeof(msg_id);
	}

	return true;
}

bool CServerForGS::add_message_handle( e_msg_gs_as msgID, func_message_handle handle )
{
	if (m_mapHandle[msgID])
	{
		printf("already exsit handle for %u", msgID);
		return false;
	}

	m_mapHandle[msgID] = handle;
	return true;
}

void CServerForGS::async_cb( uv_async_t* handle )
{
	ASSERT(handle);
	stGSAsyncData* asyncData = (stGSAsyncData*)handle->data;
	ASSERT(asyncData);
	//do something
	switch (asyncData->op)
	{
	case EGSAsyncOP_Enter_Area_Req:
		{
			
			as_gs_enter_area_req req;

			BYTE client_id_buf[GUID_BYTE_LEN] = {0};
			asyncData->client_id.ToBytes(client_id_buf, GUID_BYTE_LEN);
			req.set_client_id(client_id_buf, GUID_BYTE_LEN);
			
			printf("\n 2   \n");

			//BYTE* token = (BYTE*)asyncData->data;
			//req.set_token(token, GUID_BYTE_LEN);
			//delete[] token;

			Guid* token = (Guid*)asyncData->data;
			ASSERT(token);
			BYTE token_buf[GUID_BYTE_LEN] = {0};
			token->ToBytes(token_buf, GUID_BYTE_LEN);
			delete token;
			req.set_token(token_buf, GUID_BYTE_LEN);

			req.set_user_id(asyncData->user_id);

			CServerForGS::GetInstance()->Send(asyncData->area_id, e_msg_as_gs_enter_area_req, &req);
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

bool CServerForGS::Send( UINT32 area_id, UINT32 msg_id, google::protobuf::Message* pMessage )
{
	std::map<UINT32, CArea*>::iterator it = m_mapAreaID2Area.find(area_id);
	if (it != m_mapAreaID2Area.end())
	{
		CArea* pArea = it->second;
		ASSERT(pArea);

		uv_tcp_t* pconnect = get_client(pArea->area_id);
		if (pconnect)
		{
			return send((uv_stream_t*)pconnect, msg_id, pMessage);
		}
		else{
			printf("CServerForGS::Send 失败！ 找不到area_id[%d]的链接\n", area_id);
			return false;
		}
	}
	else{
		printf("CServerForGS::Send 失败！ 找不到area_id[%d]的链接\n", area_id);
		return false;
	}
}

bool CServerForGS::Send( Guid client_id, UINT32 msg_id, google::protobuf::Message* pMessage )
{
	uv_tcp_t* pconnect = get_client(client_id);
	if (pconnect)
	{
		return send((uv_stream_t*)pconnect, msg_id, pMessage);
	}
	else{
		printf("找不到client_id的链接");
		return false;
	}
}

CArea* CServerForGS::GetAreaByAreaID( UINT32 area_id )
{
	std::map<UINT32, CArea*>::iterator it = m_mapAreaID2Area.find(area_id);
	if (it != m_mapAreaID2Area.end())
	{
		return it->second;
	}
	return NULL;
}

bool CServerForGS::handle_msg_enter_area_res( uv_stream_t* stream, const char * buf, int len )
{
	ASSERT(stream);
	gs_as_enter_area_res* res = new gs_as_enter_area_res;
	res->ParseFromArray(buf, len);

	typedef struct  
	{
		gs_as_enter_area_res * res;
		UINT32 area_id;
	} enter_area_res_info;

	enter_area_res_info* info = new enter_area_res_info;
	info->res = res;

	CGSContext* ctx = (CGSContext*)stream->data;
	ASSERT(ctx);
	info->area_id = ctx->area_id;

	stUserAsyncData* asyncData = new stUserAsyncData;
	asyncData->op = EUserAsyncOP_Enter_Area;
	//asyncData->client_id = res->client_id();//注意！！这种得不到正确数据！
	//asyncData->client_id = *(res->mutable_client_id());//注意！！这种得不到正确数据！
	asyncData->client_id = (BYTE*)(res->mutable_client_id()->c_str());

	asyncData->data = info;

	uv_async_t* async = CServerForUser::GetInstance()->CreateAsync();
	async->data = asyncData;

	int r = uv_async_send(async);
	ASSERT(r == 0);
	if (r != 0)
	{
		PRINTF_UV_ERROR(r);
		delete res;
		delete info;
		delete asyncData;
		async->data = NULL;
		CServerForUser::GetInstance()->DestroyAsync(async);
	}

	return r == 0;
}