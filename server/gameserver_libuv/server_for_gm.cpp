#include "server_for_gm.h"

//#include "area.h"
//#include "str.h"
//#include "db_sync_operator.h"

#include "libuv_helper.h"
#include "net_config.h"

//#include "db_mgr.h"
#include "game_config.h"
#include "server_for_user.h"
bool CServerForGM::Init()
{
	ASSERT(add_message_handle(e_msg_gm_gs::e_msg_gm_gs_add_item_req, handle_msg_add_item_req));
	ASSERT(add_message_handle(e_msg_gm_gs::e_msg_gm_gs_marquee_req, handle_msg_marquee_req));
	ASSERT(add_message_handle(e_msg_gm_gs::e_msg_gm_gs_mail_req, handle_msg_mail_req));
	ASSERT(add_message_handle(e_msg_gm_gs::e_msg_gm_gs_arena_mail_req, handle_msg_arena_mail_req));
	return true;
}

uv_async_t* CServerForGM::CreateAsync()
{
	ASSERT(m_loop);
	uv_async_t* async = (uv_async_t*)malloc(sizeof uv_async_t);
	ASSERT(async);
	int r = uv_async_init(m_loop, async, async_cb);
	ASSERT(r == 0);
	return async;
}

void CServerForGM::DestroyAsync( uv_async_t* handle )
{
	ASSERT(handle);
	free(handle);
}

Guid CServerForGM::add_client( uv_tcp_t* client )
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


	Guid connect_id = GetGuidGenerator().newGuid();
	ASSERT(m_clients.find(connect_id) == m_clients.end());
	m_clients[connect_id] = client;

	CGMContext* ctx = new CGMContext;
	ctx->client_id = connect_id;
	//ctx->stauts = area::EAreaStatus::area_EAreaStatus_EAreaStatus_None;
	client->data = ctx;

	printf("CServerForGM::add_client 链接进来GS的链接！ip:port  [%s:%d]   \n", ip, sock_addr.sin_port);
	return connect_id;
}

bool CServerForGM::remove_client( Guid key )
{
	
	std::map<Guid, uv_tcp_t*>::iterator it =  m_clients.find(key);
	if (it != m_clients.end())
	{
		ASSERT(it->second);
		CGMContext* ctx = (CGMContext*)it->second->data;
		ASSERT(ctx);
		//ctx->stauts = area::EAreaStatus::area_EAreaStatus_EAreaStatus_None;

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

uv_tcp_t* CServerForGM::get_client( Guid& key )
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

bool CServerForGM::Start()
{
	int r = uv_thread_create(&m_thread, run, this);
	return r == 0;
}

bool CServerForGM::Join()
{
	int r = uv_thread_join(&m_thread);
	return r == 0;
}

void CServerForGM::run( void* arg )
{
	struct sockaddr_in addr;
	int r;

	CServerForGM* server_for_gs = (CServerForGM*)arg;
	ASSERT(server_for_gs);

	server_for_gs->m_loop = (uv_loop_t*)malloc(sizeof uv_loop_t);
	uv_loop_init(server_for_gs->m_loop);

	const ServerForGMConfig& cfg = GameConfig::GetInstance()->GetServerForGMConfig();
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

	printf("ServerForGM start success! listen %s:%d \n", ip, port);
	r = uv_run(server_for_gs->m_loop, UV_RUN_DEFAULT);
	ASSERT(0 && "ServerForGM exit!");

	uv_loop_close(server_for_gs->m_loop);
	free(server_for_gs->m_loop);
}

void CServerForGM::on_client_connection( uv_stream_t* stream, int status )
{
	int r;
	struct sockaddr sockname, peername;
	int namelen;

	ASSERT(stream);
	ASSERT(status == 0);
	CServerForGM* server_for_gs = (CServerForGM*)stream->data;
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

	//如果不是本机 则不允许链接
	if ( std::string(ip) != "127.0.0.1"){
		printf("拒绝gm客户端[%s]链接！ gm客户端只允许在服务器本机上\n", ip);
		handle->data = NULL;
		
		//不可以shutdown 因为此时还不可写！ <--- 待定
		uv_shutdown_t* req = (uv_shutdown_t*)req_alloc();
		r = uv_shutdown(req, (uv_stream_t*)handle, after_client_shutdown);
		ASSERT(r == 0);

		//uv_close((uv_handle_t*)handle, after_client_close);
		return;
	}
	
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

	CServerForGM::GetInstance()->add_client(handle);
}

void CServerForGM::on_receive( uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf )
{
	ASSERT(stream);

	int r;
	//todo net_protect作一些限制处理
	if (nread > 0){
		CServerForGM::GetInstance()->process_msg(stream, buf->base, nread);
		free(buf->base);//释放内存
	}
	else if (nread == 0)//OK, but nothing read
	{

	}
	else{

		//ASSERT(uv_last_error(stream->loop).code == UV_EOF);

		
		CGMContext* ctx = (CGMContext*)stream->data;
		ASSERT(ctx);

		if (nread == UV_EOF) {
			printf("[%p] gm链接主动断开\n", stream);

		} else if (nread == UV_ECONNRESET) {
			printf("[%p] gm链接异常断开\n", stream);

		} else {
			//printf("areaid[%d] gs链接异常 %s : %s \n", ctx->area_id, uv_err_name(nread), uv_strerror(nread));
			printf("[%p] gm链接异常 \n", stream);
			PRINTF_UV_ERROR(nread);
		}
		uv_shutdown_t* req = (uv_shutdown_t*)req_alloc();
		r = uv_shutdown(req, stream, after_client_shutdown);
		ASSERT(r == 0);
	}
}

void CServerForGM::after_client_shutdown( uv_shutdown_t* req, int status )
{
	ASSERT(req);
	ASSERT(req->handle);
	uv_close((uv_handle_t*)req->handle, after_client_close);
	req_free((uv_req_t*)req);
}

void CServerForGM::after_client_close( uv_handle_t* handle )
{
	//uv_tcp_t* client = (uv_tcp_t*)handle;
	//ASSERT(client);

	ASSERT(handle);
	CGMContext* ctx = (CGMContext*)handle->data;
	//ASSERT(ctx);
	//delete client;
	if (ctx){
		ASSERT(CServerForGM::GetInstance()->remove_client(ctx->client_id));
	}
	else{
		uv_tcp_t* client = (uv_tcp_t*)handle;
		delete client;
	}
}

bool CServerForGM::send( uv_stream_t* stream, UINT32 msg_id, google::protobuf::Message* pMessage/*, uv_write_cb write_cb*/ )
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
		printf("CServerForGM::send msgid[%d]  bytes len[%d] \n", msg_id, PACKET_HEAD_LEN + body_len);
		return true;
	}
}

void CServerForGM::after_send( uv_write_t* req, int status )
{
	ASSERT(req != NULL);
	if (status == 0)
	{
		//write_cb_called++;
		printf("CServerForGM::after_send 给GS发送信息成功\n");
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

bool CServerForGM::process_msg( uv_stream_t* stream, const char * buf, int len )
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
		ASSERT(msg_id > e_msg_gm_gs::e_msg_gm_gs_min && msg_id < e_msg_gm_gs::e_msg_gm_gs_max);
		p += sizeof msg_id;

		func_message_handle pMsgHandle = m_mapHandle[msg_id];
		if (pMsgHandle)
		{
			bool r = pMsgHandle(stream, p, msg_len - sizeof(msg_len) - sizeof(msg_id));
		}
		else{
			printf("CServerForGM::process_msg没有找到msdid[%d][%s]的消息处理函数\n", msg_id, e_msg_gm_gs_Name(e_msg_gm_gs(msg_id)).c_str());
			//return false;
		}

		p += msg_len - sizeof(msg_len) - sizeof(msg_id);
	}

	return true;
}

bool CServerForGM::add_message_handle( e_msg_gm_gs msgID, func_message_handle handle )
{
	if (m_mapHandle[msgID])
	{
		printf("already exsit handle for %u", msgID);
		return false;
	}

	m_mapHandle[msgID] = handle;
	return true;
}

void CServerForGM::SendAsync( Guid& sender_client_id, stServerForGMAsync::EAsyncOP eAsyncOP, google::protobuf::Message* data )
{
	stServerForGMAsync* asyncData = new stServerForGMAsync();
	asyncData->op = eAsyncOP;
	asyncData->client_id = sender_client_id;
	asyncData->data = data;

	uv_async_t* async = CServerForGM::GetInstance()->CreateAsync();
	async->data = asyncData;

	int r = uv_async_send(async);
	ASSERT(r == 0);
	if (r != 0)
	{
		PRINTF_UV_ERROR(r);
		delete data;
		delete asyncData;
		async->data = NULL;
		CServerForGM::GetInstance()->DestroyAsync(async);
	}
}
void CServerForGM::async_cb( uv_async_t* handle )
{
	ASSERT(handle);
	stServerForGMAsync* asyncData = (stServerForGMAsync*)handle->data;
	ASSERT(asyncData);
	//do something
	//switch (asyncData->op)
	//{
	//case EGSAsyncOP_Enter_Area_Req:
	//	{
	//		
	//		as_gs_enter_area_req req;

	//		BYTE client_id_buf[GUID_BYTE_LEN] = {0};
	//		asyncData->client_id.ToBytes(client_id_buf, GUID_BYTE_LEN);
	//		req.set_client_id(client_id_buf, GUID_BYTE_LEN);
	//		
	//		printf("\n 2   \n");

	//		//BYTE* token = (BYTE*)asyncData->data;
	//		//req.set_token(token, GUID_BYTE_LEN);
	//		//delete[] token;

	//		Guid* token = (Guid*)asyncData->data;
	//		ASSERT(token);
	//		BYTE token_buf[GUID_BYTE_LEN] = {0};
	//		token->ToBytes(token_buf, GUID_BYTE_LEN);
	//		delete token;
	//		req.set_token(token_buf, GUID_BYTE_LEN);

	//		req.set_user_id(asyncData->user_id);

	//		CServerForGM::GetInstance()->Send(asyncData->area_id, e_msg_as_gs_enter_area_req, &req);
	//	}break;

	//}

	switch (asyncData->op)
	{
	case stServerForGMAsync::EAsyncOP_Marquee_Res:
		{
			gs_gm_marquee_res* res = (gs_gm_marquee_res*)asyncData->data;
			ASSERT(res);

			CServerForGM::GetInstance()->Send(asyncData->client_id, e_msg_gs_gm_marquee_res, res);
			
			delete res;
		}
		break;
	case stServerForGMAsync::EAsyncOP_Mail_Res:
		{
			gs_gm_mail_res* res = (gs_gm_mail_res*)asyncData->data;
			ASSERT(res);

			CServerForGM::GetInstance()->Send(asyncData->client_id, e_msg_gs_gm_mail_res, res);

			delete res;
		}
		break;
	case stServerForGMAsync::EAsyncOP_Add_Item_Res:
		{
			gs_gm_add_item_res* res = (gs_gm_add_item_res*)asyncData->data;
			ASSERT(res);

			CServerForGM::GetInstance()->Send(asyncData->client_id, e_msg_gs_gm_add_item_res, res);

			delete res;
		}
		break;
	case stServerForGMAsync::EAsyncOP_Arena_Mail_Res:
		{
			gs_gm_arena_mail_res* res = (gs_gm_arena_mail_res*)asyncData->data;
			ASSERT(res);

			CServerForGM::GetInstance()->Send(asyncData->client_id, e_msg_gs_gm_arena_mail_res, res);

			delete res;
		}
		break;
	}

	delete asyncData;
	uv_close((uv_handle_t*)handle, after_async_close);
}

void CServerForGM::after_async_close( uv_handle_t* handle )
{
	ASSERT(handle != NULL);
	//free((uv_async_t*)handle);
	CServerForGM::GetInstance()->DestroyAsync((uv_async_t*)handle);
}

bool CServerForGM::Send( Guid client_id, UINT32 msg_id, google::protobuf::Message* pMessage )
{
	uv_tcp_t* pconnect = get_client(client_id);
	if (pconnect)
	{
		return send((uv_stream_t*)pconnect, msg_id, pMessage);
	}
	else{
		printf("消息[%u]发送失败，找不到client_id的链接\n", msg_id);
		return false;
	}
}

//e_gsgm_errorcode CServerForGM::check( uv_stream_t* stream, e_msg_c_gs req_msg_id, google::protobuf::Message* pInReq, const void* buf, int len)
//{
//	ASSERT(stream);
//	bool r = pInReq->ParseFromArray(buf, len);
//	if (!r)
//	{
//		PRINTF_PBPARSE_ERROR(req_msg_id);
//		return e_gsc_errorcode_unknown_error;
//	}
//}

bool CServerForGM::handle_msg_add_item_req( uv_stream_t* stream, const char * buf, int len )
{
	e_msg_gm_gs req_msg_id = e_msg_gm_gs_add_item_req;
	e_msg_gs_gm res_msg_id = e_msg_gs_gm_add_item_res;
	gm_gs_add_item_req* req = new gm_gs_add_item_req;
	gs_gm_add_item_res res;

	bool r = req->ParseFromArray(buf, len);
	if (!r){
		res.set_error_code(e_gsgm_errorcode_unkown_error);
		send(stream, res_msg_id, &res);
		return false;
	}

	CGMContext* ctx = get_context(stream);
	ASSERT(ctx);
	CServerForUser::GetInstance()->SendAsync(ctx->client_id, CServerForUserAsync::EAsyncOP_GM_Add_Item, req);

	return false;
}

bool CServerForGM::handle_msg_marquee_req( uv_stream_t* stream, const char * buf, int len )
{
	e_msg_gm_gs req_msg_id = e_msg_gm_gs_marquee_req;
	e_msg_gs_gm res_msg_id = e_msg_gs_gm_marquee_res;
	gm_gs_marquee_req* req = new gm_gs_marquee_req;
	gs_gm_marquee_res res;

	bool r = req->ParseFromArray(buf, len);
	if (!r){
		res.set_error_code(e_gsc_errorcode_unknown_error);
		send(stream, res_msg_id, &res);
		return false;
	}

	CGMContext* ctx = get_context(stream);
	ASSERT(ctx);
	CServerForUser::GetInstance()->SendAsync(ctx->client_id, CServerForUserAsync::EAsyncOP_GM_Marquee, req);

	return true;
}

bool CServerForGM::handle_msg_mail_req( uv_stream_t* stream, const char * buf, int len )
{
	e_msg_gm_gs req_msg_id = e_msg_gm_gs_mail_req;
	e_msg_gs_gm res_msg_id = e_msg_gs_gm_mail_res;
	gm_gs_mail_req* req = new gm_gs_mail_req;
	gs_gm_mail_res res;

	bool r = req->ParseFromArray(buf, len);
	if (!r){
		res.set_error_code(e_gsc_errorcode_unknown_error);
		send(stream, res_msg_id, &res);
		return false;
	}

	CGMContext* ctx = get_context(stream);
	ASSERT(ctx);
	CServerForUser::GetInstance()->SendAsync(ctx->client_id, CServerForUserAsync::EAsyncOP_GM_Mail, req);

	return true;
}

bool CServerForGM::handle_msg_arena_mail_req( uv_stream_t* stream, const char * buf, int len )
{
	e_msg_gm_gs req_msg_id = e_msg_gm_gs_arena_mail_req;
	e_msg_gs_gm res_msg_id = e_msg_gs_gm_arena_mail_res;

	gm_gs_arena_mail_req* req = new gm_gs_arena_mail_req;
	gs_gm_mail_res res;

	bool r = req->ParseFromArray(buf, len);
	if (!r)
	{
		res.set_error_code(e_gsgm_errorcode_unkown_error);
		send(stream, res_msg_id, &res);
		return false;
	}
	CGMContext* ctx = get_context(stream);
	ASSERT(ctx);
	CServerForUser::GetInstance()->SendAsync(ctx->client_id, CServerForUserAsync::EAsyncOP_GM_Arena_Mail, req);

	return true;
}
