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

/** Plugin id : type-author-name (to guarantee uniqueness) */
#define GMCHESS_NETWORK_PLUGIN_ID "gmchess-network-game"

static void send_gmchess(const char *mv);


static void ok_poune(const char *m)
{
	//ok，start the gmchess game
	send_gmchess("network-game");

}

static void no_poune(const char *m)
{
	//no, do nothing

}

static void send_gmchess(const char *mv)
{
	char *cmd;
	cmd = g_strdup_printf("gmchess %s", mv);
	system(cmd);
	g_free(cmd);
}

static gboolean
writing_im_msg_cb(PurpleAccount * account, const char *who, char **buffer,
		  PurpleConversation * conv, PurpleMessageFlags flags,
		  void *data)
{
	gchar **wrk;
	wrk = g_strsplit(*buffer, ",", -1);
	g_assert(wrk);

	if (strstr(wrk[0], "[{game:gmchess,") != NULL) {

		if (strstr(wrk[2], "action:ask") != NULL) {
			if (strstr(wrk[3], "status:start")
			    != NULL) {
				char *ask;
				ask =
				    g_strdup_printf
				    (" %s is asking you play gmchess together!",
				     who);
				purple_request_action
				    ("gmchess ask",
				     "gmchess ask", ask,
				     "Do you want to play the game?",
				     0, account, who, conv,
				     "test", 2, "Yes",
				     ok_poune, "No", no_poune);
				g_free(ask);
			} else if (strstr(wrk[3], "status:rue") != NULL) {
			} else if (strstr(wrk[3], "status:draw") != NULL) {
			} else if (strstr(wrk[3], "status:lose") != NULL) {

			}
		} else if (strstr(wrk[2], "action:reply")
			   != NULL) {

		} else if (strstr(wrk[2], "action:working")
			   != NULL) {
			send_gmchess(wrk[6]);
			/*
			   char * moves;
			   char* mv;
			   moves=g_strdup_printf("bestmoves:%s",mv);
			   send_gmchess(moves);
			   g_free(moves);
			 */
		}
		return TRUE;
	} else {
		return FALSE;
	}
}

static void
gmchess_button_cb(GtkButton * button, PidginConversation * gtkconv)
{

	gchar *joinstr;
	guint32 session_id_;
	session_id_ = g_random_int();
	joinstr =
	    g_strdup_printf
	    ("[{game:gmchess,id:%X,action:ask,status:start}]",
	     session_id_);
	gtk_imhtml_append_text(GTK_IMHTML(gtkconv->entry), joinstr, FALSE);
	g_signal_emit_by_name(gtkconv->entry, "message_send");
	g_free(joinstr);
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
	gtk_box_pack_start(GTK_BOX(conv->toolbar), button,
			   FALSE, FALSE, 0);
	g_object_set_data(G_OBJECT(conv->toolbar),
			  "gmchess_separator", sep);
	g_object_set_data(G_OBJECT(conv->toolbar),
			  "gmchess_button", button);

}

static void remove_gmchess_button_pidgin(PidginConversation * gtkconv)
{

	GtkWidget *gmchess_button = NULL, *separator = NULL;

	gmchess_button =
	    g_object_get_data(G_OBJECT(gtkconv->toolbar),
			      "gmchess_button");
	if (gmchess_button != NULL) {
		gtk_container_remove(GTK_CONTAINER
				     (gtkconv->toolbar), gmchess_button);
		g_signal_handlers_disconnect_by_func
		    (G_OBJECT(gmchess_button),
		     G_CALLBACK(gmchess_button_cb), gtkconv);
		gtk_widget_destroy(gmchess_button);
	}

	separator =
	    g_object_get_data(G_OBJECT(gtkconv->toolbar),
			      "gmchess_separator");
	if (gmchess_button != NULL) {
		gtk_container_remove(GTK_CONTAINER
				     (gtkconv->toolbar), separator);
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
	while (convs) {
		PurpleConversation *conv =
		    (PurpleConversation *) convs->data;
		/* Setup gmchess button */
		if (PIDGIN_IS_PIDGIN_CONVERSATION(conv)) {
			create_gmchess_button_pidgin
			    (PIDGIN_CONVERSATION(conv));
		}

		convs = convs->next;
	}

	purple_signal_connect(conv_handle,
			      "writing-im-msg", plugin,
			      PURPLE_CALLBACK(writing_im_msg_cb), NULL);

	purple_debug(PURPLE_DEBUG_INFO, "plugins",
		     "gmchess support plugin loaded.\n");
	return TRUE;
}

static gboolean plugin_unload(PurplePlugin * plugin)
{
	GList *convs = purple_get_conversations();
	while (convs) {
		PurpleConversation *conv =
		    (PurpleConversation *) convs->data;

		if (PIDGIN_IS_PIDGIN_CONVERSATION(conv)) {
			remove_gmchess_button_pidgin
			    (PIDGIN_CONVERSATION(conv));
		}
		convs = convs->next;
	}
	purple_debug(PURPLE_DEBUG_INFO, "plugins",
		     "gmchess support plugin unloaded.\n");

	return TRUE;
}

static PurplePluginInfo info = {
	PURPLE_PLUGIN_MAGIC,
	PURPLE_MAJOR_VERSION,
	PURPLE_MINOR_VERSION,
	PURPLE_PLUGIN_STANDARD,				  /**< type           */
	NULL,						  /**< ui_requirement */
	0,						  /**< flags          */
	NULL,						  /**< dependencies   */
	PURPLE_PRIORITY_DEFAULT,			  /**< priority       */

	GMCHESS_NETWORK_PLUGIN_ID,				   /**< id             */
	N_("gmchess network play game support."),				  /**< name           */
	"1.0",							/**< version        */
							  /**  summary        */
	N_("gmchess network play game support."),
							  /**  description    */
	N_("play chinese game in which use pidgin."),
	"lerosua <lerosua@gmail.com>",
					      /**< author         */
	"http://github.com/gmcore/pidgin-gmchess/",					/**< homepage       */

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
