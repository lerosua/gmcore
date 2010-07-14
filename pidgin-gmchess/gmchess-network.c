/* GPLv2 license */

#define PURPLE_PLUGINS

#include <glib.h>
#include <string.h>
#include "internal.h"
#include "debug.h"
#include "plugin.h"
#include "signals.h"
#include "version.h"
#include "pidgin.h"
#include "gtkconv.h"
#include "gtkconvwin.h"
#include "gtkimhtml.h"
#include "account.h"
#include "request.h"
#include "conversation.h"
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>

/** Plugin id : type-author-name (to guarantee uniqueness) */
#define GMCHESS_NETWORK_PLUGIN_ID "gmchess-network-game"
#define GMVERSION "version:0.01"
#define GMPORT  20126

enum { QUES_NO = 0, QUES_PLAY, QUES_DRAW, QUES_RUE };
/**
 * role : 0 is black, 1 is red.
 *
 * gmstatus 里的ask值:
 * ask : 0 则无问题
 *	 1 请求开局
 *	 2 请求和棋
 *	 3 请求悔棋
 */

typedef struct gmstatus_ {
    guint32 id;
    int role;
    int number;
    int ask;
    int respond;
    guint timeout_id;
    PidginConversation *conv;
} gmstatus;

static void send_gmchess(const char *mv);
static guint32 get_session_id(const gchar * idstr);
static gboolean timeout_call(gpointer data);
GIOChannel *io_channel;
int source_id;
int fd;
static gmstatus _global_status;
//_global_status.timeout_id=0;



static void init_gm_status()
{
    _global_status.id = 0;
    _global_status.role = 0;
    _global_status.number = 0;
    _global_status.ask = 0;
    _global_status.respond = 0;
	if (_global_status.timeout_id) {
		g_source_remove(_global_status.timeout_id);
		_global_status.timeout_id = 0;
	}
    _global_status.conv = NULL;

}

static void gtk_info_msg(const gchar * msg)
{
    //gchar* header;
    //GtkIMHtmlOptions options = GTK_IMHTML_NO_COLOURS;
    //header = g_strdup_printf(_("<b> %s </b><br><hr>"),msg);
    //gtk_imhtml_append_text(GTK_IMHTML(_global_status.conv),header,options);
    //purple_notify_error(NULL,NULL,"info",header);
    //g_free(header);
    purple_notify_info(NULL, "pidgin-gmchess", _("info"), msg);

}

static guint32 get_session_id(const gchar * idstr)
{
    guint32 _id;
    if (strstr(idstr, "id:") == NULL)
	return 0;
    gchar **getid;
    getid = g_strsplit(idstr, ":", -1);
    g_assert(getid);
    _id = g_ascii_strtoll(getid[1], NULL, 16);
    g_strfreev(getid);
    return _id;

}

static int create_socket()
{
    int sockfd;
    struct sockaddr_in srvaddr;

    if (-1 == (sockfd = socket(AF_INET, SOCK_STREAM, 0)))
	return -1;
    bzero(&srvaddr, sizeof(srvaddr));
    srvaddr.sin_family = AF_INET;
    srvaddr.sin_port = htons(GMPORT);
    srvaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int on = 1;
    if (-1 ==
	(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))))
	return -1;
    if (bind(sockfd, (struct sockaddr *) &srvaddr,
	     sizeof(struct sockaddr)) == -1) {
	return -1;

    }
    if (-1 == listen(sockfd, 128))
	return -1;
    return sockfd;
}

gboolean read_socket(GIOChannel * source, GIOCondition condition,
		     gpointer data)
{
    int fd_cli;
    fd_cli = -1;
    if (-1 == (fd_cli = accept(fd, NULL, 0)));
    char buf[1024];
    size_t len = 0;
    len = read(fd_cli, &buf[0], 1023);
    buf[len] = 0;
    gchar *joinstr;
    if (len > 0) {
		if (_global_status.conv) {
			purple_debug(PURPLE_DEBUG_INFO, "plugins",
					"gmchess send %s.\n",buf);
			gchar *enemy_name = g_strdup_printf("%s",
					_global_status.conv->
					active_conv->name);
			gchar *my_name = g_strdup_printf("%s",
					_global_status.conv->
					active_conv->account->
					username);
			if (strstr(buf, "moves:") != NULL) {
				_global_status.number++;
				joinstr =
					g_strdup_printf
					("[{game:gmchess,id:%X,action:working,status:NULL,role:%d,number:%d,%s,enemy_name:%s,my_name:%s}]",
					 _global_status.id,
					 _global_status.role,
					 _global_status.number, buf, enemy_name, my_name);
				gtk_imhtml_append_text(GTK_IMHTML
						(_global_status.conv->entry),
						joinstr, FALSE);
				g_signal_emit_by_name(_global_status.conv->entry,
						"message_send");
				g_free(joinstr);
			} else if (strstr(buf, "resign") != NULL) {
				joinstr =
					g_strdup_printf
					("[{game:gmchess,id:%X,action:ask,status:lose,role:%d,number:%d,moves:NULL,enemy_name:%s,my_name:%s}]",
					 _global_status.id,
					 _global_status.role,
					 _global_status.number, enemy_name, my_name);
				gtk_imhtml_append_text(GTK_IMHTML
						(_global_status.conv->entry),
						joinstr, FALSE);
				g_signal_emit_by_name(_global_status.conv->entry,
						"message_send");
				g_free(joinstr);
				init_gm_status();


			} else if (strstr(buf, "close") != NULL) {
				joinstr =
					g_strdup_printf
					("[{game:gmchess,id:%X,action:ask,status:close,role:%d,number:%d,moves:NULL,enemy_name:%s,my_name:%s}]",
					 _global_status.id,
					 _global_status.role,
					 _global_status.number, enemy_name, my_name);
				gtk_imhtml_append_text(GTK_IMHTML
						(_global_status.conv->entry),
						joinstr, FALSE);
				g_signal_emit_by_name(_global_status.conv->entry,
						"message_send");
				g_free(joinstr);
				init_gm_status();

			} else if (strstr(buf, "rue") != NULL) {
				joinstr =
					g_strdup_printf
					("[{game:gmchess,id:%X,action:ask,status:rue,role:%d,number:%d,moves:NULL,enemy_name:%s,my_name:%s}]",
					 _global_status.id,
					 _global_status.role,
					 _global_status.number, enemy_name, my_name);
				gtk_imhtml_append_text(GTK_IMHTML
						(_global_status.conv->entry),
						joinstr, FALSE);
				g_signal_emit_by_name(_global_status.conv->entry,
						"message_send");
				g_free(joinstr);

			} else if (strstr(buf, "draw") != NULL) {
				joinstr =
					g_strdup_printf
					("[{game:gmchess,id:%X,action:ask,status:draw,role:%d,number:%d,moves:NULL,enemy_name:%s,my_name:%s}]",
					 _global_status.id,
					 _global_status.role,
					 _global_status.number, enemy_name, my_name);
				gtk_imhtml_append_text(GTK_IMHTML
						(_global_status.conv->entry),
						joinstr, FALSE);
				g_signal_emit_by_name(_global_status.conv->entry,
						"message_send");
				g_free(joinstr);
			}
			g_free(enemy_name);
			g_free(my_name);
		}

    }
    return TRUE;


}

// check the pacake is valid
static gboolean check_package(const char *buf)
{
    if (strstr(buf, "[{game:gmchess,") == NULL)
		return FALSE;
    if (strstr(buf, ",id:") == NULL)
		return FALSE;
    if (strstr(buf, ",action:") == NULL)
		return FALSE;
    if (strstr(buf, ",status:") == NULL)
		return FALSE;
    if (strstr(buf, ",role:") == NULL)
		return FALSE;
    if (strstr(buf, ",number:") == NULL)
		return FALSE;
    if (strstr(buf, ",moves:") == NULL)
		return FALSE;
    if (strstr(buf, ",enemy_name:") == NULL)
		return FALSE;
    if (strstr(buf, ",my_name:") == NULL)
		return FALSE;
    if (strstr(buf, "}]") == NULL)
		return FALSE;
    return TRUE;
}

static void ok_poune(const char *m)
{
    //ok，start the gmchess game
    //send_gmchess("network-game-black");
    _global_status.role = 0;
    _global_status.number = 0;
    //then send reply to parter
    gchar *joinstr;
    gchar *enemy_name =
	g_strdup_printf("%s", _global_status.conv->active_conv->name);
    gchar *my_name = g_strdup_printf("%s",
				     _global_status.conv->
				     active_conv->account->username);
    switch(_global_status.ask){
	    case QUES_PLAY:
		    {
			    gchar *snd_str;
			    snd_str =
				    g_strdup_printf("network-game-black,enemy_name:%s,my_name:%s",
						    enemy_name, my_name);
			    send_gmchess(snd_str);
			    g_free(snd_str);

			    joinstr =
				    g_strdup_printf
				    ("[{game:gmchess,id:%X,action:reply,status:ok,role:%d,number:%d,moves:NULL,enemy_name:%s,my_name:%s}]",
				     _global_status.id, _global_status.role, _global_status.number,
				     enemy_name, my_name);
			    gtk_imhtml_append_text(GTK_IMHTML(_global_status.conv->entry),
					    joinstr, FALSE);
			    g_signal_emit_by_name(_global_status.conv->entry, "message_send");
		    }
		    break;
	    case QUES_DRAW:
		    {
			    joinstr =
				    g_strdup_printf
				    ("[{game:gmchess,id:%X,action:reply,status:ok,role:%d,number:%d,moves:NULL,enemy_name:%s,my_name:%s}]",
				     _global_status.id, _global_status.role, _global_status.number,
				     enemy_name, my_name);
			    gtk_imhtml_append_text(GTK_IMHTML(_global_status.conv->entry),
					    joinstr, FALSE);
			    g_signal_emit_by_name(_global_status.conv->entry, "message_send");

			    send_gmchess("network-game-draw");
		    }
		    break;
	    case QUES_RUE:
			{
			    joinstr =
				    g_strdup_printf
				    ("[{game:gmchess,id:%X,action:reply,status:ok,role:%d,number:%d,moves:NULL,enemy_name:%s,my_name:%s}]",
				     _global_status.id, _global_status.role, _global_status.number,
				     enemy_name, my_name);
			    gtk_imhtml_append_text(GTK_IMHTML(_global_status.conv->entry),
					    joinstr, FALSE);
			    g_signal_emit_by_name(_global_status.conv->entry, "message_send");

			    send_gmchess("network-game-rue");
		    }
		    break;
    }

    g_free(joinstr);
    g_free(enemy_name);
    g_free(my_name);

}

static void no_poune(const char *m)
{
    //no, do nothing

    gchar *joinstr;
    gchar *enemy_name =
	g_strdup_printf("%s", _global_status.conv->active_conv->name);
    gchar *my_name = g_strdup_printf("%s",
				     _global_status.conv->
				     active_conv->account->username);
    joinstr =
	g_strdup_printf
	("[{game:gmchess,id:%X,action:reply,status:no,role:%d,number:%d,moves:NULL,enemy_name:%s,my_name:%s}]",
	 _global_status.id, _global_status.role, _global_status.number,
	 enemy_name, my_name);
    gtk_imhtml_append_text(GTK_IMHTML(_global_status.conv->entry), joinstr,
			   FALSE);
    g_signal_emit_by_name(_global_status.conv->entry, "message_send");
    g_free(joinstr);
    g_free(enemy_name);
    g_free(my_name);

}

static void send_gmchess(const char *mv)
{
    gchar *argv[3];
    argv[0] = "gmchess";
    argv[1] = (gchar *) mv;
    argv[2] = NULL;
    GError *err;
    GSpawnFlags flas = (GSpawnFlags) (G_SPAWN_SEARCH_PATH |
				      G_SPAWN_STDOUT_TO_DEV_NULL |
				      G_SPAWN_STDERR_TO_DEV_NULL);
    g_spawn_async(NULL, argv, NULL, flas, NULL, NULL, NULL, &err);

}

static gboolean
writing_im_msg_cb(PurpleAccount * account, const char *who, char **buffer,
		  PurpleConversation * conv, PurpleMessageFlags flags,
		  void *data)
{
    gchar **wrk;

    //if ((strstr(*buffer, "[{game:gmchess,") != NULL)&&(strstr(*buffer,"}]")!=NULL)) {
    if (check_package(*buffer)) {
	wrk = g_strsplit(*buffer, ",", -1);
	g_assert(wrk);
	gchar *my_name;
	my_name = g_strdup_printf("my_name:%s", account->username);
	if (strstr(wrk[8], my_name) != NULL) {
	    purple_debug(PURPLE_DEBUG_INFO, "plugins",
			    "receive the package of my %s\n", *buffer);
	    g_free(my_name);
	    return TRUE;
	}
	g_free(my_name);


	if (strstr(wrk[2], "action:ask") != NULL) {
	    if (strstr(wrk[3], "status:start")
		!= NULL) {

		_global_status.conv = PIDGIN_CONVERSATION(conv);
		if (strstr(*buffer, "version:0.01") == NULL) {

		    gtk_info_msg
			(_("gmchess for pidgin plugins version not matching!"));
		    return TRUE;
		}
		_global_status.id = get_session_id(wrk[1]);
		_global_status.ask = QUES_PLAY;

		char *ask;
		ask =
		    g_strdup_printf
		    (" %s is asking you play gmchess together!", who);
		purple_request_action
		    ("gmchess ask",
		     "gmchess ask", ask,
		     "Do you want to play the game?",
		     0, account, who, conv,
		     "test", 2, "Yes", ok_poune, "No", no_poune);
		g_free(ask);
	    } else if (strstr(wrk[3], "status:rue") != NULL) {
		_global_status.ask = QUES_RUE;
		char *ask;
		ask =
		    g_strdup_printf
		    (" %s is asking you to rue a move!", who);
		purple_request_action
		    ("gmchess ask",
		     "gmchess ask", ask,
		     "Do you agree rue move for him?",
		     0, account, who, conv,
		     "test", 2, "Yes", ok_poune, "No", no_poune);
		g_free(ask);
	    } else if (strstr(wrk[3], "status:draw") != NULL) {
		_global_status.ask = QUES_DRAW;
		char *ask;
		ask =
		    g_strdup_printf
		    (" %s is asking you to draw the game!", who);
		purple_request_action
		    ("gmchess ask",
		     "gmchess ask", ask,
		     "Do you agree draw?",
		     0, account, who, conv,
		     "test", 2, "Yes", ok_poune, "No", no_poune);
		g_free(ask);
	    } else if (strstr(wrk[3], "status:lose") != NULL) {
		//对方认输。结束此局
		send_gmchess("network-game-win");
		init_gm_status();


	    } else if (strstr(wrk[3], "status:close") != NULL) {
		//对方关闭棋局，先当输的处理
		send_gmchess("network-game-win");
		init_gm_status();
	    }
	} else if (strstr(wrk[2], "action:reply")
		   != NULL) {
	    if (strstr(wrk[3], "status:ok") != NULL) {
		gchar *snd_str;
		switch (_global_status.ask) {
		case QUES_NO:
		    break;
		case QUES_PLAY:
		    //answer the start play
		    _global_status.respond = 1;
		    snd_str =
			g_strdup_printf
			("network-game-red,%s,%s", wrk[7], wrk[8]);
		    send_gmchess(snd_str);

		    if (_global_status.timeout_id) {
			g_source_remove(_global_status.timeout_id);
			_global_status.timeout_id = 0;
		    }
		    g_free(snd_str);
		    _global_status.ask = QUES_NO;
		    _global_status.role = 1;
		    //gtk_info_msg("network gmchess start");

		    break;
		case QUES_RUE:
		    send_gmchess("network-game-rue");
		    _global_status.ask = QUES_NO;
		    _global_status.role = 1;
		    break;
		case QUES_DRAW:
		    send_gmchess("network-game-draw");
		    _global_status.ask = QUES_NO;
		    _global_status.role = 1;
		    break;
		};

	    } else if (strstr(wrk[3], "status:no") != NULL) {
		switch (_global_status.ask) {
		case 0:
		    break;
		case QUES_PLAY:
		    //answer the start play
		    init_gm_status();
		    gtk_info_msg(_("The other side deny the invite"));

		    break;
		case QUES_RUE:
		    send_gmchess("network-game-norue");
		    _global_status.ask = QUES_NO;
		    _global_status.role = 1;
		    break;
		case QUES_DRAW:
		    send_gmchess("network-game-nodraw");
		    _global_status.ask = QUES_NO;
		    _global_status.role = 1;
		    break;
		};

	    }

	} else if (strstr(wrk[2], "action:working")
		   != NULL) {
	    purple_debug(PURPLE_DEBUG_INFO, "plugins",
			    "should send the gmchess'moves: %s\n", *buffer);
	    send_gmchess(wrk[6]);
	    /*
	       char * moves;
	       char* mv;
	       moves=g_strdup_printf("bestmoves:%s",mv);
	       send_gmchess(moves);
	       g_free(moves);
	     */
	} else {
	    g_strfreev(wrk);
	    return FALSE;

	}
	g_strfreev(wrk);
	return TRUE;
    } else {
	return FALSE;
    }
}

static gboolean timeout_call(gpointer data)
{
    if (1 == _global_status.respond && 1 == _global_status.ask) {

    } else {
	purple_debug(PURPLE_DEBUG_INFO, "plugins",
		     "send the ask start game not respond,give up it.\n");
	gtk_info_msg(_("there has no respond. the other side maybe not install the gmchess plugins"));
	init_gm_status();

    }
    return FALSE;
}

static void
gmchess_button_cb(GtkButton * button, PidginConversation * gtkconv)
{
	/** 如果id不为0,则可能在下棋中。退出*/
	if (_global_status.id != 0) {
		gtk_info_msg(_("Please do not repeat invite. "));
		return;

	}
    //gtk_info_msg("发送邀请中");
    gchar *enemy_name = g_strdup_printf("%s", gtkconv->active_conv->name);
    gchar *my_name =
	g_strdup_printf("%s", gtkconv->active_conv->account->username);

    gchar *joinstr;
    guint32 session_id_;
    session_id_ = g_random_int();
    joinstr =
	g_strdup_printf
	("[{game:gmchess,id:%X,action:ask,status:start,role:0,number:0,moves:NULL,enemy_name:%s,my_name:%s,version:0.01}]",
	 session_id_, enemy_name, my_name);
    gtk_imhtml_append_text(GTK_IMHTML(gtkconv->entry), joinstr, FALSE);
    g_signal_emit_by_name(gtkconv->entry, "message_send");
    purple_debug(PURPLE_DEBUG_INFO, "plugins",
			    "send joinstr: %s\n", joinstr);
    g_free(joinstr);
    g_free(enemy_name);
    g_free(my_name);

    _global_status.id = session_id_;
    _global_status.ask = 1;
    _global_status.conv = gtkconv;

    _global_status.timeout_id =
	g_timeout_add_seconds(30, timeout_call, NULL);
}

static void create_gmchess_button_pidgin(PidginConversation * conv)
{
    GtkWidget *button, *image, *sep;
    //GtkIconInfo *icoinfo;
    //const gchar *iconames[]={"gmchess-pidgin", "gmchess-pidgin"};
    button = gtk_button_new();
    gtk_button_set_relief(GTK_BUTTON(button), GTK_RELIEF_NONE);
    g_signal_connect(G_OBJECT(button), "clicked",
		     G_CALLBACK(gmchess_button_cb), conv);
#ifdef DATADIR
    //image = gtk_image_new_from_file(g_build_filename(DATADIR,"pixmaps","hicolor","22x22","apps","gmchess.png",NULL));
    //image = gtk_image_new_from_file(g_build_filename("/usr/share","icons","hicolor","22x22","apps","gmchess.png",NULL));
    image =
	gtk_image_new_from_file
	("/usr/share/icons/hicolor/22x22/apps/gmchess.png");
#else
    //image = gtk_image_new_from_file(g_build_filename("/usr/share","icons","hicolor","22x22","apps","gmchess.png",NULL));
    image =
	gtk_image_new_from_file
	("/usr/share/icons/hicolor/22x22/apps/gmchess.png");
#endif
    gtk_container_add((GtkContainer *) button, image);
    sep = gtk_vseparator_new();
    gtk_widget_show(sep);
    gtk_widget_show(image);
    gtk_widget_show(button);
    gtk_box_pack_start(GTK_BOX(conv->toolbar), sep, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(conv->toolbar), button, FALSE, FALSE, 0);
    g_object_set_data(G_OBJECT(conv->toolbar), "gmchess_separator", sep);
    g_object_set_data(G_OBJECT(conv->toolbar), "gmchess_button", button);

}

static void remove_gmchess_button_pidgin(PidginConversation * gtkconv)
{

    GtkWidget *gmchess_button = NULL, *separator = NULL;

    gmchess_button =
	g_object_get_data(G_OBJECT(gtkconv->toolbar), "gmchess_button");
	if (gmchess_button != NULL) {
		gtk_container_remove(GTK_CONTAINER
				(gtkconv->toolbar), gmchess_button);
		g_signal_handlers_disconnect_by_func
			(G_OBJECT(gmchess_button),
			 G_CALLBACK(gmchess_button_cb), gtkconv);
		gtk_widget_destroy(gmchess_button);
	}

    separator =
	g_object_get_data(G_OBJECT(gtkconv->toolbar), "gmchess_separator");
	if (gmchess_button != NULL) {
		gtk_container_remove(GTK_CONTAINER(gtkconv->toolbar), separator);
		gtk_widget_destroy(separator);
	}
}

static gboolean plugin_load(PurplePlugin * plugin)
{
    GList *convs = purple_get_conversations();
    void *conv_handle = purple_conversations_get_handle();
    void *gtk_conv_handle = pidgin_conversations_get_handle();

    purple_signal_connect(gtk_conv_handle,
			  "conversation-displayed",
			  plugin,
			  PURPLE_CALLBACK
			  (create_gmchess_button_pidgin), NULL);
    purple_signal_connect(gtk_conv_handle,
			  "conversation-hiding",
			  plugin,
			  PURPLE_CALLBACK
			  (remove_gmchess_button_pidgin), NULL);
	while (convs) {
		PurpleConversation *conv = (PurpleConversation *) convs->data;
		/* Setup gmchess button */
		if (PIDGIN_IS_PIDGIN_CONVERSATION(conv)) {
			create_gmchess_button_pidgin(PIDGIN_CONVERSATION(conv));
		}

		convs = convs->next;
	}

    purple_signal_connect(conv_handle,
			  "writing-im-msg", plugin,
			  PURPLE_CALLBACK(writing_im_msg_cb), NULL);

    purple_debug(PURPLE_DEBUG_INFO, "plugins",
		 "gmchess support plugin loaded.\n");
    //int fd;
    fd = create_socket();
    if (fd == -1)
		purple_debug(PURPLE_DEBUG_INFO, "plugins",
		     "gmchess pidgin bind socket error\n");
	else {
		io_channel = g_io_channel_unix_new(fd);
		g_io_channel_set_encoding(io_channel, NULL, NULL);
		source_id = g_io_add_watch(io_channel, G_IO_IN, read_socket, &fd);

	}
    init_gm_status();
    return TRUE;
}

static gboolean plugin_unload(PurplePlugin * plugin)
{
    GList *convs = purple_get_conversations();
	while (convs) {
		PurpleConversation *conv = (PurpleConversation *) convs->data;

		if (PIDGIN_IS_PIDGIN_CONVERSATION(conv)) {
			remove_gmchess_button_pidgin(PIDGIN_CONVERSATION(conv));
		}
		convs = convs->next;
	}
    purple_debug(PURPLE_DEBUG_INFO, "plugins",
		 "gmchess support plugin unloaded.\n");
	if (fd != -1) {
		g_source_remove(source_id);
		g_io_channel_shutdown(io_channel, TRUE, NULL);
		g_io_channel_unref(io_channel);
	}
    init_gm_status();

    return TRUE;
}

static PurplePluginInfo info = {
    PURPLE_PLUGIN_MAGIC,
    PURPLE_MAJOR_VERSION,
    PURPLE_MINOR_VERSION,
    PURPLE_PLUGIN_STANDARD,				  /**< type           */
    NULL,						  /**< ui_requirement */
    0,							  /**< flags          */
    NULL,						  /**< dependencies   */
    PURPLE_PRIORITY_DEFAULT,				  /**< priority       */

    GMCHESS_NETWORK_PLUGIN_ID,					   /**< id             */
    N_("gmchess network play game support."),					  /**< name           */
    "0.01",							/**< version        */
							  /**  summary        */
    N_("gmchess network play game support."),
							  /**  description    */
    N_("play chinese game in which use pidgin."),
    "lerosua <lerosua@gmail.com>",
					      /**< author         */
    "http://github.com/gmcore/pidgin-gmchess/",						/**< homepage       */

    plugin_load,					  /**< load           */
    plugin_unload,					  /**< unload         */
    NULL,						  /**< destroy        */

    NULL,						  /**< ui_info        */
    NULL,						  /**< extra_info     */
    NULL,
    NULL,
    /* Padding */
    NULL,
    NULL,
    NULL,
    NULL
};

static void init_plugin(PurplePlugin * plugin)
{
}

PURPLE_INIT_PLUGIN(pidgin - gmchess, init_plugin, info)
