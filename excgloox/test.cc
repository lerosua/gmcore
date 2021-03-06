#include <gloox/client.h>
#include <gloox/messagesessionhandler.h>
#include <gloox/messageeventhandler.h>
#include <gloox/messageeventfilter.h>
#include <gloox/chatstatehandler.h>
#include <gloox/chatstatefilter.h>
#include <gloox/connectionlistener.h>
#include <gloox/disco.h>
#include <gloox/message.h>
#include <gloox/gloox.h>
#include <gloox/lastactivity.h>
#include <gloox/loghandler.h>
#include <gloox/logsink.h>
#include <gloox/connectiontcpclient.h>
#include <gloox/connectionsocks5proxy.h>
#include <gloox/connectionhttpproxy.h>
#include <gloox/messagehandler.h>
#include "excgloox.h"
using namespace gloox;

#ifndef _WIN32
# include <unistd.h>
#endif

#include <stdio.h>
#include <string>

#include <cstdio> // [s]print[f]

#if defined( WIN32 ) || defined( _WIN32 )
# include <windows.h>
#endif



class MessageTest : public MessageSessionHandler, ConnectionListener, LogHandler,
                    MessageEventHandler, MessageHandler, ChatStateHandler
{
  public:
    MessageTest() : m_session( 0 ), m_messageEventFilter( 0 ), m_chatStateFilter( 0 ) 
		    ,handle_message(NULL)
		    ,logou_result(NULL)
		    {}

    virtual ~MessageTest() {}

    void init_func(void(*func1_)(),void(*func2_)(const char*,const char*) )
    {
	    logou_result = func1_;
	    handle_message=func2_;


    }

    void start()
    {

      JID jid( "botcalk@gmail.com/gloox" );
      j = new Client( jid, "botcalk2038" );
      j->registerConnectionListener( this );
      j->registerMessageSessionHandler( this, 0 );
      j->disco()->setVersion( "messageTest", GLOOX_VERSION, "Linux" );
      j->disco()->setIdentity( "client", "bot" );
      j->disco()->addFeature( XMLNS_CHAT_STATES );
      StringList ca;
      ca.push_back( "/path/to/cacert.crt" );
      j->setCACerts( ca );

      j->logInstance().registerLogHandler( LogLevelDebug, LogAreaAll, this );

//
// this code connects to a jabber server through a SOCKS5 proxy
//
//       ConnectionSOCKS5Proxy* conn = new ConnectionSOCKS5Proxy( j,
//                                   new ConnectionTCP( j->logInstance(),
//                                                      "sockshost", 1080 ),
//                                   j->logInstance(), "example.net" );
//       conn->setProxyAuth( "socksuser", "sockspwd" );
//       j->setConnectionImpl( conn );

//
// this code connects to a jabber server through a HTTP proxy through a SOCKS5 proxy
//
//       ConnectionTCP* conn0 = new ConnectionTCP( j->logInstance(), "old", 1080 );
//       ConnectionSOCKS5Proxy* conn1 = new ConnectionSOCKS5Proxy( conn0, j->logInstance(), "old", 8080 );
//       conn1->setProxyAuth( "socksuser", "sockspwd" );
//       ConnectionHTTPProxy* conn2 = new ConnectionHTTPProxy( j, conn1, j->logInstance(), "jabber.cc" );
//       conn2->setProxyAuth( "httpuser", "httppwd" );
//       j->setConnectionImpl( conn2 );


      if( j->connect( false ) )
      {
        ConnectionError ce = ConnNoError;
        while( ce == ConnNoError )
        {
          ce = j->recv();
        }
        printf( "ce: %d\n", ce );
      }

      delete( j );
    }

    virtual void onConnect()
    {
      printf( "connected!!!\n" );
      logou_result();
      
    }

    virtual void onDisconnect( ConnectionError e )
    {
      printf( "message_test: disconnected: %d\n", e );
      if( e == ConnAuthenticationFailed )
        printf( "auth failed. reason: %d\n", j->authError() );
    }

    virtual bool onTLSConnect( const CertInfo& info )
    {
      time_t from( info.date_from );
      time_t to( info.date_to );

      printf( "status: %d\nissuer: %s\npeer: %s\nprotocol: %s\nmac: %s\ncipher: %s\ncompression: %s\n"
              "from: %s\nto: %s\n",
              info.status, info.issuer.c_str(), info.server.c_str(),
              info.protocol.c_str(), info.mac.c_str(), info.cipher.c_str(),
              info.compression.c_str(), ctime( &from ), ctime( &to ) );
      return true;
    }

    virtual void handleMessage( const Message& msg, MessageSession * /*session*/ )
    {

      //printf( "type: %d, subject: %s, message: %s, thread id: %s\n", msg.subtype(),
       //       msg.subject().c_str(), msg.body().c_str(), msg.thread().c_str() );

	    handle_message(msg.from().full().c_str(),msg.body().c_str());
      std::string re = "You said:\n> " + msg.body() + "\nI like that statement.";
      std::string sub;
      if( !msg.subject().empty() )
        sub = "Re: " +  msg.subject();

      m_messageEventFilter->raiseMessageEvent( MessageEventDisplayed );
#if defined( WIN32 ) || defined( _WIN32 )
      Sleep( 1000 );
#else
      sleep( 1 );
#endif
      m_messageEventFilter->raiseMessageEvent( MessageEventComposing );
      m_chatStateFilter->setChatState( ChatStateComposing );
#if defined( WIN32 ) || defined( _WIN32 )
      Sleep( 2000 );
#else
      sleep( 2 );
#endif
      m_session->send( re, sub );

      if( msg.body() == "quit" )
        j->disconnect();
    }

    virtual void handleMessageEvent( const JID& from, MessageEventType event )
    {
      printf( "received event: %d from: %s\n", event, from.full().c_str() );
    }

    virtual void handleChatState( const JID& from, ChatStateType state )
    {
      printf( "received state: %d from: %s\n", state, from.full().c_str() );
    }

    virtual void handleMessageSession( MessageSession *session )
    {
      printf( "got new session\n");
      // this example can handle only one session. so we get rid of the old session
      j->disposeMessageSession( m_session );
      m_session = session;
      m_session->registerMessageHandler( this );
      m_messageEventFilter = new MessageEventFilter( m_session );
      m_messageEventFilter->registerMessageEventHandler( this );
      m_chatStateFilter = new ChatStateFilter( m_session );
      m_chatStateFilter->registerChatStateHandler( this );
    }

    virtual void handleLog( LogLevel level, LogArea area, const std::string& message )
    {
      //printf("log: level: %d, area: %d, %s\n", level, area, message.c_str() );
    }

  private:
    Client *j;
    MessageSession *m_session;
    MessageEventFilter *m_messageEventFilter;
    ChatStateFilter *m_chatStateFilter;
    void (*logou_result)();
    void (*handle_message)(const char* ,const char*);
};

#if 0
int main( int /*argc*/, char** /*argv*/ )
{
  MessageTest *r = new MessageTest();
  r->start();
  delete( r );
  return 0;
}
#endif

MessageTest *qx_gloox = NULL;

BOOL qx_init(void)
{
  qx_gloox = new MessageTest();

}
void qx_register(void(*func1)(void),void (*func2)(const char* ,const char* ))
{

	qx_gloox->init_func(func1,func2);

}

void qx_connect()
{
	printf("connection in c\n");
}
void qx_recv_mesg()
{
	printf("recv msg in c \n");

}

void qx_terminate(void)
{
	delete(qx_gloox);
	qx_gloox=NULL;

}

BOOL qx_login(const char* user_id, const char* passwd_,BOOL block)
{
	qx_gloox->start();


}
BOOL qx_logout(const char* user_id)
{

}

BOOL qx_send_message(unsigned long uid, const char * msg, BOOL block)
{

}

BOOL qx_change_state_buddy(unsigned long uid, long state)
{

}
