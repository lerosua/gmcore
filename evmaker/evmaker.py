#!/usr/bin/env python
# -*- coding: UTF-8 -*-

import sys
import os
from urllib import quote
from urllib import unquote
try:
    import pygtk
    pygtk.require("2.0")
except:
    pass

try:
    import gtk
except:
    sys.exit(1)


COL_NAME = 0
COL_PIXBUF = 1
COL_PATH = 2
COL_PIXBUF_BIG = 3

evhome_dir = "/tmp"
video_preview_jpg = "/tmp/00000001.jpg"

class EvMakerApp():
    def __init__(self):
        self.builder = gtk.Builder()
        self.builder.add_from_file("evmaker.glade")
        self.window = self.builder.get_object("main_window")
        self.window.set_default_size(1024,600)
        self.window.set_position(gtk.WIN_POS_CENTER)
        self.window.set_title("Easy Video Maker")
        self.window.connect("destroy", gtk.main_quit)

        self.current_dir = "/home/leros/"
        self.fileIcon = self.get_icon(gtk.STOCK_FILE)
        self.dirIcon = self.get_icon(gtk.STOCK_OPEN)

        self.store_src  = self.create_store_src()
        #self.fill_store()
        #iconview_src put the source of video
        self.iconview_src = self.builder.get_object("iconview_src")
        self.iconview_src.set_model(self.store_src)
        self.iconview_src.set_reorderable(1)
        self.iconview_src.set_selection_mode(gtk.SELECTION_SINGLE)
        self.iconview_src.set_orientation(gtk.ORIENTATION_VERTICAL)
        self.iconview_src.set_columns(20)
        self.iconview_src.set_item_width(90)
        self.iconview_src.set_text_column(COL_NAME)
        self.iconview_src.set_pixbuf_column(COL_PIXBUF)
        self.iconview_src.connect("item-activated", self.on_src_item_activated)
        self.iconview_src.connect("selection-changed", self.on_src_item_selection_changed)

        self.ipcTargets = [('STRING',0,0)]
        self.iconview_src.drag_source_set(gtk.gdk.BUTTON1_MASK, self.ipcTargets, gtk.gdk.ACTION_COPY)
        self.iconview_src.connect("drag_data_get", self.drag_data_src_get)

        self.all_vbox = self.builder.get_object("all_vbox")
        self.all_vbox.drag_dest_set(gtk.DEST_DEFAULT_DROP | gtk.DEST_DEFAULT_MOTION, self.ipcTargets, gtk.gdk.ACTION_COPY)
        self.all_vbox.connect("drag_data_received", self.drag_data_src_received)
        #self.iconview_dst = self.builder.get_object("iconview_dst")

        # tool buttons
        self.bt_open = self.builder.get_object("bt_open")
        self.bt_open.connect("clicked", self.on_bt_open_clicked)
        self.bt_load = self.builder.get_object("bt_load")
        self.bt_load.connect("clicked", self.on_bt_load_clicked)


        self.preview_widget = self.builder.get_object("image_view")
        self.bt_play = self.builder.get_object("bt_play")
        self.bt_play.connect("clicked", self.on_bt_play_clicked)



        self.window.show_all()


    def create_store_src(self):
        store = gtk.ListStore(str,gtk.gdk.Pixbuf,str,gtk.gdk.Pixbuf)
        return store
        
    def fill_store(self):
        self.store_src.clear()
        for fl in os.listdir(self.current_dir):
            if not fl[0] == '.':
                if os.path.isdir(os.path.join(self.current_dir,fl)):
                    self.store_src.append([fl,self.dirIcon,fl,self.dirIcon])
                else:
                    self.store_src.append([fl,self.fileIcon,fl,self.fileIcon])

    def get_icon(self,name):
        theme = gtk.icon_theme_get_default()
        return theme.load_icon(name,48,0)

    def on_src_item_activated(self,widget, item):
        model = widget.get_model()
        path = model[item][COL_PATH]
        print "click ", path
        #os.system("mplayer "+path)
        #self.run("mplayer",path)
        self.preview(path)

    def on_src_item_selection_changed(self, widget):
        model = widget.get_model()
        selected = widget.get_selected_items()
        if len(selected) == 0:
            return
        item = selected[0][0]
        icon = model[item][COL_PIXBUF_BIG]
        self.preview_widget.set_from_pixbuf(icon)
        print model[item][COL_PATH]



    def on_bt_open_clicked(self, widget):
        print "open clicked"

    def on_bt_load_clicked(self, widget):
        print "load clicked"
        fn_widget = gtk.FileChooserDialog("Select a Video File",None,gtk.FILE_CHOOSER_ACTION_OPEN,buttons=(gtk.STOCK_CANCEL,gtk.RESPONSE_CANCEL, gtk.STOCK_OPEN,gtk.RESPONSE_OK))
        fn_widget.set_local_only(True)
        fn_filter = gtk.FileFilter()
        fn_filter.set_name("video/*")
        fn_filter.add_mime_type("video/*")
        fn_widget.add_filter(fn_filter)
        fn_filter_all = gtk.FileFilter()
        fn_filter_all.set_name("all file")
        fn_filter_all.add_pattern("*")
        fn_widget.add_filter(fn_filter_all)
        if fn_widget.run() == gtk.RESPONSE_OK:
            self.load_src_file(fn_widget.get_filename())
        fn_widget.destroy()

    def on_bt_play_clicked(self, widget):
        model = self.iconview_src.get_model()
        selected = self.iconview_src.get_selected_items()
        if len(selected) == 0:
            return
        item = selected[0][0]
        filename = model[item][COL_PATH]
        self.preview(filename)

    def load_src_file(self, filename):
        os.chdir(evhome_dir)
        os.system("mplayer -ss 90 -noframedrop -nosound -vo jpeg -frames 1 "+filename)
        tmpicon = gtk.gdk.pixbuf_new_from_file_at_size(video_preview_jpg,96,96)
        tmpicon_preview = gtk.gdk.pixbuf_new_from_file(video_preview_jpg)
        name = os.path.basename(filename)
        self.store_src.append([name,tmpicon,filename,tmpicon_preview])

    def drag_data_src_get(self,widget, context, selection_data, info, timestamp):
        print "drag get"

    def drag_data_src_received(self, widget, context, x, y, selection, targetType, timestamp):
        print "drog receive"
        filename = selection.data.strip()
        tmp = unquote(filename.strip('[\']'))
        self.load_src_file(tmp[7:])

    def preview(self,*args):
        self.run("mplayer","-osdlevel","3",*args)
    
    def run(self,program, *args):
        pid = os.fork()
        if not pid:
            os.execvp(program,(program,)+args)
        return os.wait()[0]

       
if __name__ == "__main__":
    app = EvMakerApp()
    gtk.main()

