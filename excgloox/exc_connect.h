/*
* =====================================================================================
*
*       Filename:  TalkConnect.h
*
*    Description:  连接处理类
*
*        Version:  1.0
*        Created:  2007年07月1日 16时52分02秒 CST
*       Revision:  none
*       Compiler:  gcc
*
*         Author:  lerosua (), lerosua@gmail.com
*        Company:  Cyclone
*
* =====================================================================================
*/
#ifndef _TALKCONNECT_H_
#define _TALKCONNECT_H_

#include <gloox/connectionlistener.h>
#include <gloox/loghandler.h>
#include <iostream>

using namespace gloox;

using namespace std;


/**
 * @brief 监听网络连接的类
 *
 */

class TalkConnect: public ConnectionListener
{

public:
        TalkConnect();
        virtual ~TalkConnect();

protected:
        virtual void onConnect();
        virtual void onDisconnect(ConnectionError er);
        virtual void onResourceBindError(ResourceBindError error);
        virtual void onSessionCreateError(SessionCreateError error);
        virtual bool onTLSConnect(const CertInfo& info);
        virtual void onStreamEvent(StreamEvent event);

};



#endif
