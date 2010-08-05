#!/usr/bin/env python
# -*- coding: UTF-8 -*-

import sys
import os
import pygtk
pygtk.require("2.0")
import gtk
from urllib import quote
from urllib import unquote
from markscale import MarkScale
from player import player


COL_NAME = 0
COL_PIXBUF = 1
COL_PATH = 2
COL_PIXBUF_BIG = 3
COL_INFO = 4

evhome_dir = "/tmp/"
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
        self.player = player()
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

        #for timeline
        adj = gtk.Adjustment(0.0,0.0,100.0,0.1,1.0,1.0)
        self.timeline = MarkScale(adj)
        vbox_timeline = self.builder.get_object("vbox_timeline")
        self.timeline.set_draw_value(False)
        vbox_timeline.pack_start(self.timeline,False, False)
        # tool buttons
        self.builder.get_object("bt_open").connect("clicked", self.on_bt_open_clicked)
        self.builder.get_object("bt_load").connect("clicked", self.on_bt_load_clicked)
        self.builder.get_object("bt_delete").connect("clicked", self.on_bt_delete_clicked)
        self.builder.get_object("bt_quit").connect("clicked",self.on_bt_quit_clicked)
        self.builder.get_object("bt_split").connect("clicked", self.on_bt_split_clicked)
        self.builder.get_object("bt_merge").connect("clicked", self.on_bt_merge_clicked)

        self.label_A = self.builder.get_object("label_A_time")
        self.label_B = self.builder.get_object("label_B_time")



        self.preview_image = self.builder.get_object("image_view")
        pix = gtk.gdk.pixbuf_new_from_file_at_size(app_jpg,300,200)
        self.preview_image.set_from_pixbuf(pix)

        self.builder.get_object("bt_play").connect("clicked", self.on_bt_play_clicked)
        self.builder.get_object("bt_a").connect("clicked", self.on_bt_a_clicked)
        self.builder.get_object("bt_b").connect("clicked", self.on_bt_b_clicked)


        self.window.show_all()


    def create_store(self):
        # filename, preview jpg, filepath, big jpg, fileinfo [length,width,height]
        store = gtk.ListStore(str, gtk.gdk.Pixbuf, str, gtk.gdk.Pixbuf, str)
        return store
        
    def get_icon(self,name):
        theme = gtk.icon_theme_get_default()
        return theme.load_icon(name,48,0)

    def on_src_item_activated(self,widget, item):
        model = widget.get_model()
        path = model[item][COL_PATH]
        self.player.preview(path)

    def on_src_item_selection_changed(self, widget):
        self.iconview_dst.unselect_all()
        model = widget.get_model()
        selected = widget.get_selected_items()
        if len(selected) == 0:
            return
        item = selected[0][0]
        icon = model[item][COL_PIXBUF_BIG]
        self.preview_image.set_from_pixbuf(icon)
        info = model[item][COL_INFO]
        infos = info.strip('[]').split(',')
        length = infos[0].strip('\'')
        w = infos[1].strip(' \'')
        h = infos[2].strip(' \'')
        self.timeline.setNbFrames(float(length))
        print model[item][COL_PATH]
        print infos,length, w,h



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

        a_time = self.label_A.get_text()
        b_time = self.time_to_string(self.timeline.getB() - self.timeline.getA())
        outfile = evhome_dir+"outfile_"+str(self.file_num)+".avi"
        cmd = "mencoder"+" -ss "+a_time+" -endpos "+b_time+"  -ovc copy -oac copy "+filename+" -o "+outfile
        self.file_num += 1
        print cmd
        print self.file_num
        self.wait_run(cmd)
        self.load_dst_file(outfile)

    def on_bt_merge_clicked(self,widget):
        model = self.iconview_dst.get_model()
        iter = model.get_iter_first()
        filename = ""
        size_w = 1000
        size_h = 0
        while ( iter != None ):
            row = model.get_path(iter)
            filename += " "
            filename += model[row][COL_PATH]
            info = model[row][COL_INFO]
            infos = info.strip('[]').split(',')
            print infos , infos[1]
            w = infos[1].strip(' \'')
            h = infos[2].strip(' \'')
            if int(w) < size_w :
                size_w = int(w)
                size_h = int(h)
            iter = model.iter_next(iter)
        pic_size = str(size_w)+":"+str(size_h)
        cmd = "mencoder -ovc lavc -oac mp3lame -idx -vf scale=" + pic_size + " " + filename+" -o /tmp/out.avi"
        print cmd
        self.wait_run(cmd)

    def on_bt_play_clicked(self, widget):
        model = self.iconview_src.get_model()
        selected = self.iconview_src.get_selected_items()
        if len(selected) == 0:
            return
        item = selected[0][0]
        filename = model[item][COL_PATH]
        a_time = self.label_A.get_text()
        b_time = self.time_to_string(self.timeline.getB() - self.timeline.getA())
        self.player.preview(filename,a_time,b_time)
        
    def on_bt_a_clicked(self, widget):
        val = self.timeline.get_value()
        self.timeline.setA(float('%.2f'%val))
        a = self.timeline.getA()
        self.label_A.set_text(self.time_to_string(a))

    def on_bt_b_clicked(self, widget):
        val = self.timeline.get_value()
        self.timeline.setB(float('%.2f'%val))
        b = self.timeline.getB()
        self.label_B.set_text(self.time_to_string(b))

    def load_src_file(self, filename):
        jpg = self.player.get_screenshot(filename)
        #length = self.player.get_length(filename)
        length,width,height = self.player.get_info(filename)
        tmpicon = gtk.gdk.pixbuf_new_from_file_at_size(jpg,96,96)
        tmpicon_preview = gtk.gdk.pixbuf_new_from_file_at_size(jpg,400,300)
        name = os.path.basename(filename)
        #self.store_src.append([name,tmpicon,filename,tmpicon_preview, length])
        self.store_src.append([name,tmpicon,filename,tmpicon_preview, [length,width,height]])

    def load_dst_file(self, filename):
        jpg = self.player.get_screenshot(filename)
        #length = self.player.get_length(filename)
        length,width,height = self.player.get_info(filename)
        tmpicon = gtk.gdk.pixbuf_new_from_file_at_size(jpg,96,96)
        tmpicon_preview = gtk.gdk.pixbuf_new_from_file_at_size(jpg,400,300)
        name = os.path.basename(filename)
        #self.store_dst.append([name,tmpicon,filename,tmpicon_preview, length])
        self.store_dst.append([name,tmpicon,filename,tmpicon_preview, [length,width,height]])


    def drag_data_src_get(self,widget, context, selection_data, info, timestamp):
        print "drag get"

    def drag_data_src_received(self, widget, context, x, y, selection, targetType, timestamp):
        filename = selection.data.strip()
        tmp = unquote(filename.strip('[\']'))
        self.load_src_file(tmp[7:])

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

    def time_to_string(self,value):
        """
        Converts the given time in nanoseconds to a human readable string
    
        Format HH:MM:SS
        """
        mins = value / 60
        sec = value % 60
        hours = mins / 60
        mins = mins % 60
        return "%02d:%02d:%02d" % (hours, mins, sec)


       
if __name__ == "__main__":
    app = EvMakerApp()
    gtk.main()

