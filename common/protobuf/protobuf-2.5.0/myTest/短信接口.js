	//天翼开放平台
	sms189:
	{
		appID			: "234082630000036352",	//后台应用管理查看
		appSecretKey	: "82e3d836f6d369fa02a0471e701f0938",//后台应用管理查看
		tokenUrl		: "https://oauth.api.189.cn/emp/oauth2/v3/access_token",//获取token接口
		smsUrl			: "http://api.189.cn/v2/emp/templateSms/sendSms",	//短信接口
		templeteID		: "91001602", //短信模板ID 到短信模板申请页面查看
	}





				
				//请求token
				request.post( setting.sms189.tokenUrl, 
					{form:{
							grant_type	: 'client_credentials',
							app_id		: setting.sms189.appID,
							app_secret	: setting.sms189.appSecretKey
					}},
					function (e, r, body) {
						// Ideally, you would take the body in the response
						// and construct a URL that a user clicks on (like a sign in button).
						// The verifier is only available in the response after a user has
						// verified with twitter that they are authorizing your app.
						
						if(e)
						{
							console.log(e);
							callback && callback(false, "" + e, verifyCode);
							return;
						}
						
						console.log(body);
						var access_token = JSON.parse(body);
						//console.log(access_token);

						if( !access_token || access_token["res_code"] != 0)
						{
							console.log("！！！sms获取token失败！！！");
							callback && callback(false, "！！！sms获取token失败！！！", verifyCode);
							return;
						}

						var timestamp = my_util.formatDate(new Date() , 'yyyy-MM-dd hh:mm:ss');
						//var timestamp2 =  moment().format('MMMM-Do-YYYY h:mm:ss');
						console.log(timestamp);
						//console.log(timestamp2);
						//请求发短信
						request.post( setting.sms189.smsUrl, 
							{form:{
									access_token	:	access_token["access_token"],
									app_id			:	setting.sms189.appID,
									acceptor_tel	:	phone,
									template_id		:	setting.sms189.templeteID,
									template_param	:	'{"param1" : "' +  verifyCode + '"}',
									timestamp		:	timestamp
							}},
							function (e, r, body) {
								if(e)
								{
									console.log(e);
									callback && callback(false, "" + e, verifyCode);
									return ;
								}

								console.log(body);//todo 验证是OK的
								var ret = JSON.parse(body);
								if( !ret || ret["res_code"] != 0){
									console.log("！！！sms发送失败！！！");
									callback && callback(false, "！！！sms发送失败！！！", verifyCode);
									return;
								}

								callback && callback(true, "", verifyCode);
							});
					})
					
					
					"{\"param1\" : \"nihao\"}";