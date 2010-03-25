#include <gloox/client.h>
#include <gloox/message.h>
#include <gloox/gloox.h>
#include <gloox/messagesessionhandler.h>
#include <gloox/messagehandler.h>
#include <gloox/connectionlistener.h>
#include <gloox/rostermanager.h>
#include <gloox/rosterlistener.h>
#include <gloox/disco.h>
#include <gloox/discohandler.h>
#include <gloox/presence.h>
#include "excgloox.h"

using namespace gloox;

#ifndef _WIN32
# include <unistd.h>
#endif

#include <stdio.h>
#include <string>

#include <cstdio>		// [s]print[f]

#if defined( WIN32 ) || defined( _WIN32 )
# include <windows.h>
#endif

class Excgloox:public RosterListener, MessageSessionHandler, ConnectionListener
,LogHandler	
    ,MessageHandler {
      public:
	Excgloox():m_session(0)
	, m_send_session(0)
	, handle_message(NULL)
	, handle_on_connect(NULL)
	, handle_on_disconnect(NULL)
	{
		handle_item_subscribed = NULL;
		handle_item_added = NULL;
		handle_item_unsubscribed = NULL;
		handle_item_removed = NULL;
		handle_item_updated = NULL;
		handle_roster = NULL;
		handle_roster_error = NULL;
		handle_roster_presence = NULL;
		handle_self_presence = NULL;
		handle_subscription_request = NULL;
		handle_unsubscription_request = NULL;
		handle_non_roster_presence = NULL;
	} virtual ~ Excgloox() {
	}


	virtual void onConnect() {
		handle_on_connect();

	}
	virtual bool onTLSConnect(const CertInfo & info) {
		time_t from(info.date_from);
		time_t to(info.date_to);

		printf
		    ("status: %d\nissuer: %s\npeer: %s\nprotocol: %s\nmac: %s\ncipher: %s\ncompression: %s\n"
		     "from: %s\nto: %s\n", info.status,
		     info.issuer.c_str(), info.server.c_str(),
		     info.protocol.c_str(), info.mac.c_str(),
		     info.cipher.c_str(), info.compression.c_str(),
		     ctime(&from), ctime(&to));
		return true;
	}

	virtual void onDisconnect(ConnectionError e) {
		handle_on_disconnect(e);
	}

    virtual void handleLog( LogLevel level, LogArea area, const std::string& message )
    {
    }
	virtual void handleMessage(const Message & msg,
				   MessageSession * /*session */ ) {
		handle_message(msg.from().full().c_str(),
			       msg.body().c_str());
	}

	virtual void handleMessageSession(MessageSession * session) {
		printf("got new session\n");
		// this example can handle only one session. so we get rid of the old session
		m_client->disposeMessageSession(m_session);
		m_session = session;
		m_session->registerMessageHandler(this);
	}
	void login(const char *uid, const char *passwd) {
		JID jid(uid);
		m_client = new Client(jid, passwd);
		m_client->registerConnectionListener(this);
		m_client->registerMessageSessionHandler(this, 0);
		m_client->rosterManager()->registerRosterListener(this,true);
		m_client->disco()->setVersion("Excgloox", GLOOX_VERSION,
					      "Linux");
		m_client->disco()->setIdentity("client", "GUI");

	}

	void start(unsigned int blocked) {

		m_client->connect();
		/*
		if (m_client->connect(false)) {
			ConnectionError ce = ConnNoError;
			while (ConnNoError == ce) {
				ce = m_client->recv();
			}
		}
		*/

	}
	    /** about roster manager*/
	virtual void handleItemSubscribed(const JID & jid) {
		printf("subscribed %s\n", jid.bare().c_str());
		handle_item_subscribed(jid.bare().c_str());
	}

	virtual void handleItemAdded(const JID & jid) {
		printf("added %s\n", jid.bare().c_str());
		handle_item_added(jid.bare().c_str());
	}

	virtual void handleItemUnsubscribed(const JID & jid) {
		printf("unsubscribed %s\n", jid.bare().c_str());
		handle_item_unsubscribed(jid.bare().c_str());
	}

	virtual void handleItemRemoved(const JID & jid) {
		printf("removed %s\n", jid.bare().c_str());
		handle_item_removed(jid.bare().c_str());
	}

	virtual void handleItemUpdated(const JID & jid) {
		printf("updated %s\n", jid.bare().c_str());
		handle_item_updated(jid.bare().c_str());
	}

	virtual void handleRoster(const Roster & roster) {
		printf("roster arriving\nitems:\n");
		Roster::const_iterator it = roster.begin();
		for (; it != roster.end(); ++it) {
			printf("jid: %s, name: %s, subscription: %d\n",
			       (*it).second->jid().c_str(),
			       (*it).second->name().c_str(),
			       (*it).second->subscription());
			StringList g = (*it).second->groups();
			StringList::const_iterator it_g = g.begin();
			RosterItem::ResourceMap::const_iterator rit =
			    (*it).second->resources().begin();

			handle_roster((*it).second->jid().c_str(),
				      (*it).second->name().c_str(),
				      (*it_g).c_str(),
				      (*rit).first.c_str(),
				      (*it).second->subscription());

		}
	}

	virtual void handleRosterError(const IQ & /*iq */ ) {
		printf("a roster-related error occured\n");
		handle_roster_error("error");
	}

	virtual void handleRosterPresence(const RosterItem & item,
					  const std::string & resource,
					  Presence::PresenceType presence,
					  const std::string & msg) {
		printf("presence received: %s/%s -- %d\n",
		       item.jid().c_str(), resource.c_str(), presence);

		handle_roster_presence(item.jid().c_str(),
				       resource.c_str(), msg.c_str(),
				       presence);
	}

	virtual void handleSelfPresence(const RosterItem & item,
					const std::string & resource,
					Presence::PresenceType presence,
					const std::string & msg) {
		handle_self_presence(item.jid().c_str(), resource.c_str(),
				     msg.c_str(), presence);
	}

	virtual bool handleSubscriptionRequest(const JID & jid,
					       const std::string & msg) {
		printf("subscription: %s\n", jid.bare().c_str());
		//StringList groups;
		//JID id(jid);
		//j->rosterManager()->subscribe(id, "", groups, "");
		//return true;
		return handle_subscription_request(jid.bare().c_str(),
						   msg.c_str());
	}

	virtual bool handleUnsubscriptionRequest(const JID & jid,
						 const std::string & msg) {
		printf("unsubscription: %s\n", jid.bare().c_str());
		//return true;
		return handle_unsubscription_request(jid.bare().c_str(),
						     msg.c_str());
	}

	virtual void handleNonrosterPresence(const Presence & presence) {
		printf
		    ("received presence from entity not in the roster: %s\n",
		     presence.from().full().c_str());
		handle_non_roster_presence(presence.from().full().c_str());
	}


    /** register the func */
	void register_message_handler(void (*func_)
				       (const char *, const char *)) {
		handle_message = func_;
	}
	void register_on_connect(void (*func_) (void)) {
		handle_on_connect = func_;
	}
	void register_on_disconnect(void (*func_) (int)) {
		handle_on_disconnect = func_;
	}
	void register_handle_item_subscribed(void (*func_) (const char *)) {
		handle_item_subscribed = func_;
	}
	void register_handle_item_unsubscribed(void (*func_)
					       (const char *)) {
		handle_item_unsubscribed = func_;
	}
	void register_handle_item_added(QFUNC1 func_) {
		handle_item_added = func_;
	}
	void register_handle_item_removed(QFUNC1 func_) {
		handle_item_removed = func_;
	}
	void register_handle_item_updated(QFUNC1 func_) {
		handle_item_updated = func_;
	}
	void register_handle_roster(QFUNC5 func_) {
		handle_roster = func_;
	}
	void register_handle_roster_error(QFUNC1 func_) {
		handle_roster_error = func_;
	}
	void register_handle_roster_presence(QFUNC4 func_) {
		handle_roster_presence = func_;
	}
	void register_handle_self_presence(QFUNC4 func_) {
		handle_self_presence = func_;
	}
	void register_handle_subscription_request(QFUNC2 func_) {
		handle_subscription_request = func_;
	}
	void register_handle_unsubscription_request(QFUNC2 func_) {
		handle_unsubscription_request = func_;
	}
	void register_handle_non_roster_presence(QFUNC1 func_) {
		handle_non_roster_presence = func_;
	}


      private:
	Client * m_client;
	MessageSession *m_session;
	MessageSession *m_send_session;
	void (*handle_message) (const char *, const char *);
	void (*handle_on_connect) (void);
	void (*handle_on_disconnect) (int e);

	void (*handle_item_subscribed) (const char *);
	void (*handle_item_added) (const char *);
	void (*handle_item_unsubscribed) (const char *);
	void (*handle_item_removed) (const char *);
	void (*handle_item_updated) (const char *);
	/** get the frineds func */
	void (*handle_roster) (const char *jid_, const char *nick_,
			       const char *group_, const char *resource_,
			       int subscription_);
	void (*handle_roster_error) (const char *e);
	void (*handle_roster_presence) (const char *jid_,
					const char *resource_,
					const char *msg_, int presence_);
	void (*handle_self_presence) (const char *jid_,
				      const char *resource_,
				      const char *msg_, int presence_);
	/** 收到别人加好友的请求，返回真则允许，返回假则忽略*/
	BOOL(*handle_subscription_request) (const char *jid_,
					    const char *msg_);
	BOOL(*handle_unsubscription_request) (const char *jid_,
					      const char *msg_);
	void (*handle_non_roster_presence) (const char *presence);




};

Excgloox *qx_gloox = NULL;

void qx_init(void)
{
	qx_gloox = new Excgloox();
}

void qx_login(const char *user_id, const char *passwd_)
{
	qx_gloox->login(user_id, passwd_);
}

void qx_run(BOOL block_)
{
	qx_gloox->start(block_);
}

void qx_terminate(void)
{
	delete(qx_gloox);
	qx_gloox = NULL;

}

void qx_register_handle_messgae(void (*func_) (const char *, const char *))
{
	qx_gloox->register_message_handler(func_);
}

void qx_register_handle_on_connect(void (*func_) (void))
{
	qx_gloox->register_on_connect(func_);
}

void qx_register_handle_on_disconnect(void (*func_) (int))
{
	qx_gloox->register_on_disconnect(func_);

}

void qx_register_handle_item_subscribed(void (*func_) (const char *))
{
	qx_gloox->register_handle_item_subscribed(func_);
}

void qx_register_handle_item_unsubscribed(void (*func_) (const char *))
{
	qx_gloox->register_handle_item_unsubscribed(func_);
}

void qx_register_handle_item_added(QFUNC1 func_)
{
	qx_gloox->register_handle_item_added(func_);
}

void qx_register_handle_item_removed(QFUNC1 func_)
{
	qx_gloox->register_handle_item_removed(func_);
}

void qx_register_handle_item_updated(QFUNC1 func_)
{
	qx_gloox->register_handle_item_updated(func_);
}

void qx_register_handle_roster(QFUNC5 func_)
{
	qx_gloox->register_handle_roster(func_);
}

void qx_register_handle_roster_error(QFUNC1 func_)
{
	qx_gloox->register_handle_roster_error(func_);
}

void qx_register_handle_roster_presence(QFUNC4 func_)
{
	qx_gloox->register_handle_roster_presence(func_);
}

void qx_register_handle_self_presence(QFUNC4 func_)
{
	qx_gloox->register_handle_self_presence(func_);
}

void qx_register_handle_subscription_request(QFUNC2 func_)
{
	qx_gloox->register_handle_subscription_request(func_);
}

void qx_register_handle_unsubscription_request(QFUNC2 func_)
{
	qx_gloox->register_handle_unsubscription_request(func_);
}

void qx_register_handle_non_roster_presence(QFUNC1 func_)
{
	qx_gloox->register_handle_non_roster_presence(func_);
}

BOOL qx_logout(const char *user_id)
{

}

BOOL qx_send_message(unsigned long uid, const char *msg, BOOL block)
{

}

BOOL qx_change_state_buddy(unsigned long uid, long state)
{

}
