/*
 * =====================================================================================
 * 
 *       Filename:  excgloox.h
 * 
 *    Description:  
 * 
 *        Version:  1.0
 *        Created:  2010年03月19日 11时11分03秒 CST
 *       Revision:  none
 *       Compiler:  gcc
 * 
 *         Author:  lerosua (), lerosua@gmail.com
 *        Company:  Cyclone
 * 
 * =====================================================================================
 */

#ifndef  EXCGLOOX_FILE_HEADER_INC
#define  EXCGLOOX_FILE_HEADER_INC

#ifdef __cplusplus
extern "C" {
#endif
	typedef unsigned int BOOL;
	typedef void (*QFUNC1) (const char *);
	typedef BOOL(*QFUNC2) (const char *, const char *);
	typedef void (*QFUNC4) (const char *, const char *, const char *,
				int);
	typedef void (*QFUNC5) (const char *, const char *, const char *,
				const char *, int);


	void qx_init(void);
	void qx_terminate(void);
//BOOL qx_login(const char* user_id, const char* passwd_,BOOL block);
	void qx_login(const char *user_id, const char *passwd_);
	void qx_run(BOOL block_);
	BOOL qx_logout(const char *user_id);
//发送信息
	BOOL qx_send_message(unsigned long uid, const char *msg,
			     BOOL block);

//好友在线状态改变
	BOOL qx_change_state_buddy(unsigned long uid, long state);



	void qx_connect();
	void qx_recv_mesg();
	void qx_register_handle_on_disconnect(void (*func_) (int));
	void qx_register_handle_on_connect(void (*func_) (void));
	void qx_register_handle_messgae(void (*func_)
					(const char *, const char *));
	void qx_register_handle_item_subscribed(void (*func_)
						(const char *));
	void qx_register_handle_item_unsubscribed(void (*func_)
						  (const char *));
	void qx_register_handle_item_added(QFUNC1 func_);
	void qx_register_handle_item_removed(QFUNC1 func_);
	void qx_register_handle_item_updated(QFUNC1 func_);
	void qx_register_handle_roster(QFUNC5 func_);
	void qx_register_handle_roster_error(QFUNC1 func_);
	void qx_register_handle_roster_presence(QFUNC4 func_);
	void qx_register_handle_self_presence(QFUNC4 func_);
	void qx_register_handle_subscription_request(QFUNC2 func_);
	void qx_register_handle_unsubscription_request(QFUNC2 func_);
	void qx_register_handle_non_roster_presence(QFUNC1 func_);



#ifdef __cplusplus
}
#endif
#endif				/* ----- #ifndef EXCGLOOX_FILE_HEADER_INC  ----- */
