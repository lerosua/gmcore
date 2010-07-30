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
        self.fill_store()
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
        #self.iconview_dst = self.builder.get_object("iconview_dst")
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

       
if __name__ == "__main__":
    app = EvMakerApp()
    gtk.main()

