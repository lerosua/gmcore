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
#include "conversation.h"

/** Plugin id : type-author-name (to guarantee uniqueness) */
#define GMCHESS_NETWORK_PLUGIN_ID "gmchess-network-game"

static gboolean
writing_im_msg_cb(PurpleAccount *account, const char *who, char **buffer,
				PurpleConversation *conv, PurpleMessageFlags flags, void *data)
{
		guint32 randnum;
		gchar* out;
	if ( strstr(*buffer, "[{game:gmchess,") != NULL) {
		purple_debug_misc("gmchess plugin", "chinese chess game.\n");
		randnum=g_random_int();
		out = g_strdup_printf("i receive gmchess asking session id %X",randnum);
		purple_conv_im_send_with_flags(PURPLE_CONV_IM(conv), out, flags);
		g_free(out);
		return FALSE;
	}
	else {
		return FALSE;
	}
}

static gboolean
plugin_load(PurplePlugin *plugin)
{
	void *conv_handle = purple_conversations_get_handle();
	
	purple_debug(PURPLE_DEBUG_INFO, "plugins", "gmchess support plugin loaded.\n");

	purple_signal_connect(conv_handle, "writing-im-msg", plugin, PURPLE_CALLBACK(writing_im_msg_cb), NULL);	

	return TRUE;
}

static gboolean
plugin_unload(PurplePlugin *plugin)
{
	purple_debug(PURPLE_DEBUG_INFO, "plugins", "gmchess support plugin unloaded.\n");

	return TRUE;
}

static PurplePluginInfo info =
{
	PURPLE_PLUGIN_MAGIC,
	PURPLE_MAJOR_VERSION,
	PURPLE_MINOR_VERSION,
	PURPLE_PLUGIN_STANDARD,                           /**< type           */
	NULL,                                             /**< ui_requirement */
	0,                                                /**< flags          */
	NULL,                                             /**< dependencies   */
	PURPLE_PRIORITY_DEFAULT,                          /**< priority       */

	GMCHESS_NETWORK_PLUGIN_ID,                                 /**< id             */
	N_("gmchess network play game support."),                		  /**< name           */
	"1.0",                                  		/**< version        */
	                                                  /**  summary        */
	N_("gmchess network play game support."),
	                                                  /**  description    */
	N_("play chinese game in which use pidgin."),
	"lerosua <lerosua@gmail.com>",        /**< author         */
	"http://github.com/gmcore/pidgin-gmchess/",                                     /**< homepage       */

	plugin_load,                                      /**< load           */
	plugin_unload,                                    /**< unload         */
	NULL,                                             /**< destroy        */

	NULL,                                             /**< ui_info        */
	NULL,                                             /**< extra_info     */
	NULL,
	NULL,
	/* Padding */
	NULL,
	NULL,
	NULL,
	NULL
};

static void
init_plugin(PurplePlugin *plugin)
{
}

PURPLE_INIT_PLUGIN(pidgin-gmchess, init_plugin, info)
