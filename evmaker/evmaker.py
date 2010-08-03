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

#import gmplayer

COL_NAME = 0
COL_PIXBUF = 1
COL_PATH = 2
COL_PIXBUF_BIG = 3

evhome_dir = "/tmp"
video_preview_jpg = "/tmp/00000001.jpg"
app_jpg = "evmaker.jpg"

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
        self.playing = 0
        self.file_num = 1

        self.store_src  = self.create_store()
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

        #self.iconview_src.drag_dest_set(gtk.DEST_DEFAULT_DROP | gtk.DEST_DEFAULT_MOTION, self.ipcTargets, gtk.gdk.ACTION_COPY)
        #self.iconview_src.connect("drag_data_received", self.drag_data_src_received)

        self.iconview_dst = self.builder.get_object("iconview_dst")
        self.store_dst = self.create_store()
        self.iconview_dst.set_model(self.store_dst)
        self.iconview_dst.set_reorderable(1)
        self.iconview_dst.set_selection_mode(gtk.SELECTION_SINGLE)
        self.iconview_dst.set_orientation(gtk.ORIENTATION_VERTICAL)
        self.iconview_dst.set_columns(20)
        self.iconview_dst.set_item_width(90)
        self.iconview_dst.set_text_column(COL_NAME)
        self.iconview_dst.set_pixbuf_column(COL_PIXBUF)
        self.iconview_dst.connect("item-activated", self.on_src_item_activated)

        # tool buttons
        self.bt_open = self.builder.get_object("bt_open")
        self.bt_open.connect("clicked", self.on_bt_open_clicked)
        self.bt_load = self.builder.get_object("bt_load")
        self.bt_load.connect("clicked", self.on_bt_load_clicked)
        self.bt_cut = self.builder.get_object("bt_cut")
        self.bt_cut.connect("clicked", self.on_bt_cut_clicked)
        self.bt_delete = self.builder.get_object("bt_delete")
        self.bt_delete.connect("clicked", self.on_bt_delete_clicked)
        self.bt_quit  = self.builder.get_object("bt_quit")
        self.bt_quit.connect("clicked",self.on_bt_quit_clicked)
        self.bt_split = self.builder.get_object("bt_split")
        self.bt_split.connect("clicked", self.on_bt_split_clicked)
        self.bt_merge = self.builder.get_object("bt_merge")
        self.bt_merge.connect("clicked", self.on_bt_merge_clicked)



        self.preview_image = self.builder.get_object("image_view")
        pix = gtk.gdk.pixbuf_new_from_file_at_size(app_jpg,300,200)
        self.preview_image.set_from_pixbuf(pix)
        self.bt_play = self.builder.get_object("bt_play")
        self.bt_play.connect("clicked", self.on_bt_play_clicked)


        #for preview window
        self.preview_ebox = self.builder.get_object("event_preview")
        #self.gmplayer = gmplayer.GMplayer()
        self.gmplayer = gtk.Socket()
        self.gmplayer.show()
        #self.preview_ebox.pack_start(self.gmplayer)
        

        self.window.show_all()
        #self.gmplayer.hide()


    def create_store(self):
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
        self.preview(path)

    def on_src_item_selection_changed(self, widget):
        iconview_dst.unselect_all()
        model = widget.get_model()
        selected = widget.get_selected_items()
        if len(selected) == 0:
            return
        item = selected[0][0]
        icon = model[item][COL_PIXBUF_BIG]
        self.preview_image.set_from_pixbuf(icon)
        print model[item][COL_PATH]



    def on_bt_open_clicked(self, widget):
        print "open clicked"

    def on_bt_load_clicked(self, widget):
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

    def on_bt_cut_clicked(self,widget):
        print "test"
    
    def on_bt_delete_clicked(self,widget):
        print "test"
        model = self.iconview_src.get_model()
        selected = self.iconview_src.get_selected_items()
        if len(selected) == 0:
            return
        item = selected[0][0]
        iter = model.get_iter(item)
        model.remove(iter)
        #pix = gtk.gdk.pixbuf_new_from_file(app_jpg)
        #self.preview_image.set_from_pixbuf(pix)


    def on_bt_quit_clicked(self,widget):
        gtk.main_quit()

    def on_bt_split_clicked(self, widget):
        model = self.iconview_src.get_model()
        selected = self.iconview_src.get_selected_items()
        if len(selected) == 0:
            return
        item = selected[0][0]
        filename = model[item][COL_PATH]

        start_t_h = self.builder.get_object("start_t_h").get_text()
        start_t_m = self.builder.get_object("start_t_m").get_text()
        start_t_s = self.builder.get_object("start_t_s").get_text()
        end_t_h = self.builder.get_object("end_t_h").get_text()
        end_t_m = self.builder.get_object("end_t_m").get_text()
        end_t_s = self.builder.get_object("end_t_s").get_text()
        outfile = "outfile"+str(self.file_num)+".avi"
        cmd = "mencoder"+" -ss "+start_t_h+":"+start_t_m+":"+start_t_s+" -endpos "+end_t_h+":"+end_t_m+":"+end_t_s+" -ovc copy -oac copy "+filename+" -o "+outfile
        self.file_num += 1
        print cmd
        print self.file_num
        self.wait_run(cmd)
        self.load_dst_file(outfile)

    def on_bt_merge_clicked(self,widget):
        model = self.iconview_dst.get_model()
        iter = model.get_iter_first()
        filename = ""
        while ( iter != None ):
            row = model.get_path(iter)
            filename += " "
            filename += model[row][COL_PATH]
            iter = model.iter_next(iter)
        cmd = "mencoder -ovc copy -oac copy "+filename+" -o /tmp/out.avi"
        print cmd
        self.wait_run(cmd)

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
        tmpicon_preview = gtk.gdk.pixbuf_new_from_file_at_size(video_preview_jpg,400,300)
        name = os.path.basename(filename)
        self.store_src.append([name,tmpicon,filename,tmpicon_preview])

    def load_dst_file(self, filename):
        os.chdir(evhome_dir)
        os.system("mplayer -ss 90 -noframedrop -nosound -vo jpeg -frames 1 "+filename)
        tmpicon = gtk.gdk.pixbuf_new_from_file_at_size(video_preview_jpg,96,96)
        tmpicon_preview = gtk.gdk.pixbuf_new_from_file_at_size(video_preview_jpg,400,300)
        name = os.path.basename(filename)
        self.store_dst.append([name,tmpicon,filename,tmpicon_preview])


    def drag_data_src_get(self,widget, context, selection_data, info, timestamp):
        print "drag get"

    def drag_data_src_received(self, widget, context, x, y, selection, targetType, timestamp):
        filename = selection.data.strip()
        tmp = unquote(filename.strip('[\']'))
        self.load_src_file(tmp[7:])

    def preview(self,*args):
        if self.playing == 0:
            #self.preview_ebox.remove(self.preview_image)
            #self.preview_ebox.add(self.gmplayer)
            #id = self.gmplayer.get_id()
            #self.gmplayer.set_size_request(400,300)
            self.run("mplayer","-osdlevel","3",*args)
            #self.run("mplayer","-slave","-osdlevel","3","-wid",str(id),*args)
            #self.window.show_all()
            self.playing = 1
        else:
            self.playing = 0
            #os.system("killall mplayer")
            #self.preview_ebox.remove(self.gmplayer)
            #self.preview_ebox.add(self.preview_image)
            #self.iconview_src.on_src_item_selection_changed()
    
    def run(self,program, *args):
        pid = os.fork()
        if not pid:
            os.execvp(program,(program,)+args)
        #return os.wait()[0]

    def wait_run(self,cmd):
        pid = os.fork()
        if not pid:
            os.system(cmd)
            sys.exit(0)
        return os.wait()[0]



       
if __name__ == "__main__":
    app = EvMakerApp()
    gtk.main()

