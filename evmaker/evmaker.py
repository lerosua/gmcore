#!/usr/bin/env python
# -*- coding: UTF-8 -*-

import sys
import os
try:
    import pygtk
    pygtk.require("2.0")
except:
    pass

try:
    import gtk
except:
    sys.exit(1)


COL_PATH = 0
COL_PIXBUF = 1
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
        self.iconview_src = self.builder.get_object("iconview_src")
        self.iconview_src.set_model(self.store_src)
        self.iconview_src.set_reorderable(1)
        self.iconview_src.set_selection_mode(gtk.SELECTION_SINGLE)
        self.iconview_src.set_orientation(gtk.ORIENTATION_VERTICAL)
        self.iconview_src.set_columns(20)
        self.iconview_src.set_item_width(90)
        self.iconview_src.set_text_column(COL_PATH)
        self.iconview_src.set_pixbuf_column(COL_PIXBUF)
        self.iconview_src.connect("item-activated", self.on_src_item_activated)
        self.iconview_src.connect("selection-changed", self.on_src_item_selection_changed)
        #self.iconview_dst = self.builder.get_object("iconview_dst")

        self.bt_open = self.builder.get_object("bt_open")
        self.bt_open.connect("clicked", self.on_bt_open_clicked)
        self.bt_load = self.builder.get_object("bt_load")
        self.bt_load.connect("clicked", self.on_bt_load_clicked)


        self.preview_widget = self.builder.get_object("image_view")



        self.window.show_all()


    def create_store_src(self):
        store = gtk.ListStore(str,gtk.gdk.Pixbuf)
        return store
        
    def fill_store(self):
        self.store_src.clear()
        for fl in os.listdir(self.current_dir):
            if not fl[0] == '.':
                if os.path.isdir(os.path.join(self.current_dir,fl)):
                    self.store_src.append([fl,self.dirIcon])
                else:
                    self.store_src.append([fl,self.fileIcon])

    def get_icon(self,name):
        theme = gtk.icon_theme_get_default()
        return theme.load_icon(name,48,0)

    def on_src_item_activated(self,widget, item):
        model = widget.get_model()
        path = model[item][COL_PATH]
        print "click ", path
        os.system("mplayer "+path)

    def on_src_item_selection_changed(self, widget):
        model = widget.get_model()
        selected = widget.get_selected_items()
        if len(selected) == 0:
            return
        item = selected[0][0]
        icon = model[item][COL_PIXBUF]
        self.preview_widget.set_from_pixbuf(icon)
        print model[item][COL_PATH]



    def on_bt_open_clicked(self, widget):
        print "open clicked"

    def on_bt_load_clicked(self, widget):
        print "load clicked"
        fn_widget = gtk.FileChooserDialog("Select a Video File",None,gtk.FILE_CHOOSER_ACTION_OPEN,buttons=(gtk.STOCK_CANCEL,gtk.RESPONSE_CANCEL, gtk.STOCK_OPEN,gtk.RESPONSE_OK))
        fn_widget.set_local_only(True)
        fn_filter = gtk.FileFilter()
        #fn_filter.add_pattern("*.avi")
        #fn_filter.add_pattern("*.rmvb")
        fn_filter.add_mime_type("video/*")
        fn_widget.add_filter(fn_filter)
        if fn_widget.run() == gtk.RESPONSE_OK:
            self.load_src_file(fn_widget.get_filename())
        fn_widget.destroy()

    def load_src_file(self, filename):
        os.system("mplayer -ss 1 -noframedrop -nosound -vo jpeg -frames 1 "+filename)
        tmpicon = gtk.gdk.pixbuf_new_from_file_at_size("/tmp/video.jpg",95,96)
        #self.store_src.append([filename,self.fileIcon])
        self.store_src.append([filename,tmpicon])

       
if __name__ == "__main__":
    app = EvMakerApp()
    gtk.main()

