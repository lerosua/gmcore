#!/usr/bin/env python
# -*- coding: UTF-8 -*-

import sys
import os
import pygtk
pygtk.require("2.0")
import gtk
import utils
from markscale import MarkScale
from player import player


COL_NAME = 0
COL_PIXBUF = 1
COL_PATH = 2
COL_PIXBUF_BIG = 3
COL_INFO = 4

evhome_dir = "/tmp/"
video_preview_jpg = "/tmp/00000001.jpg"
app_png = "evmaker.png"
audio_png = "audio.png"
audio_apply_png = "apply.png"

class EvMakerApp():
    def __init__(self):
        self.builder = gtk.Builder()
        self.builder.add_from_file("evmaker.glade")
        self.window = self.builder.get_object("main_window")
        self.window.set_default_size(1024,600)
        self.window.set_position(gtk.WIN_POS_CENTER)
        self.window.set_title("Easy Video Maker")
        self.window.connect("destroy", gtk.main_quit)

        #self.fileIcon = self.get_icon(gtk.STOCK_FILE)
        #self.dirIcon = self.get_icon(gtk.STOCK_OPEN)
        self.player = player()
        self.file_num = 1
        self.time_mark ="A"
        self.applyIcon =  gtk.gdk.pixbuf_new_from_file_at_size(audio_apply_png,48,48)
        self.audioIcon =  gtk.gdk.pixbuf_new_from_file_at_size(audio_png,48,48)

        self.statusbar = self.builder.get_object("statusbar")
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
        self.iconview_src.connect("item-activated", self.on_item_activated)
        self.iconview_src.connect("selection-changed", self.on_src_item_selection_changed)

        self.ipcTargets = [('STRING',0,0)]
        #self.iconview_src.drag_source_set(gtk.gdk.BUTTON1_MASK, self.ipcTargets, gtk.gdk.ACTION_COPY)
        #self.iconview_src.connect("drag_data_get", self.drag_data_src_get)

        self.vbox_src_drag = self.builder.get_object("vbox_src_drag")
        self.vbox_src_drag.drag_dest_set(gtk.DEST_DEFAULT_DROP | gtk.DEST_DEFAULT_MOTION, self.ipcTargets, gtk.gdk.ACTION_COPY)
        self.vbox_src_drag.connect("drag_data_received", self.drag_data_src_received)

        #self.iconview_src.drag_dest_set(gtk.DEST_DEFAULT_DROP | gtk.DEST_DEFAULT_MOTION, self.ipcTargets, gtk.gdk.ACTION_MOVE)
        #self.iconview_src.connect("drag_data_received", self.drag_data_src_received)

        #iconview_dst for put the split out video
        self.iconview_dst = self.builder.get_object("iconview_dst")
        self.store_dst = self.create_store()
        self.iconview_dst.set_model(self.store_dst)
        self.iconview_dst.set_reorderable(1)
        self.iconview_dst.set_selection_mode(gtk.SELECTION_MULTIPLE)
        self.iconview_dst.set_orientation(gtk.ORIENTATION_VERTICAL)
        self.iconview_dst.set_columns(20)
        self.iconview_dst.set_item_width(90)
        self.iconview_dst.set_text_column(COL_NAME)
        self.iconview_dst.set_pixbuf_column(COL_PIXBUF)
        self.iconview_dst.connect("item-activated", self.on_item_activated)

        self.vbox_dst_merge = self.builder.get_object("vbox_dst_merge")
        self.vbox_dst_merge.drag_dest_set(gtk.DEST_DEFAULT_DROP | gtk.DEST_DEFAULT_MOTION, self.ipcTargets, gtk.gdk.ACTION_COPY)
        self.vbox_dst_merge.connect("drag_data_received", self.drag_data_dst_received)

        #for put source audio file
        self.iconview_audio = self.builder.get_object("iconview_audio")
        self.store_audio = self.create_store()
        self.iconview_audio.set_model(self.store_audio)
        self.iconview_audio.set_reorderable(1)
        self.iconview_audio.set_selection_mode(gtk.SELECTION_SINGLE)
        self.iconview_audio.set_orientation(gtk.ORIENTATION_VERTICAL)
        self.iconview_audio.set_columns(20)
        self.iconview_audio.set_item_width(90)
        self.iconview_audio.set_text_column(COL_NAME)
        self.iconview_audio.set_pixbuf_column(COL_PIXBUF)
        self.iconview_audio.connect("item-activated", self.on_item_activated)
        self.iconview_audio.connect("selection-changed", self.on_audio_item_selection_changed)

        self.vbox_audio_drag = self.builder.get_object("vbox_audio_drag")
        self.vbox_audio_drag.drag_dest_set(gtk.DEST_DEFAULT_DROP | gtk.DEST_DEFAULT_MOTION, self.ipcTargets, gtk.gdk.ACTION_COPY)
        self.vbox_audio_drag.connect("drag_data_received", self.drag_data_audio_received)
        #for timeline
        adj = gtk.Adjustment(0.0,0.0,100.0,0.1,1.0,1.0)
        self.timeline = MarkScale(adj)
        vbox_timeline = self.builder.get_object("vbox_timeline")
        self.timeline.set_draw_value(False)
        vbox_timeline.pack_start(self.timeline,False, False)

        self.label_A = self.builder.get_object("label_A_time")
        self.label_B = self.builder.get_object("label_B_time")

        self.preview_image = self.builder.get_object("image_view")
        pix = gtk.gdk.pixbuf_new_from_file_at_size(app_png,300,200)
        self.preview_image.set_from_pixbuf(pix)

        # tool buttons
        self.builder.get_object("bt_about").connect("clicked", self.on_about_clicked)
        self.builder.get_object("bt_open").connect("clicked", self.on_bt_open_clicked)
        self.builder.get_object("bt_load").connect("clicked", self.on_bt_load_clicked)
        self.builder.get_object("bt_src_add").connect("clicked", self.on_bt_load_clicked)
        self.builder.get_object("bt_src_del").connect("clicked", self.on_bt_src_del_clicked)
        self.builder.get_object("bt_src_split").connect("clicked", self.on_bt_split_clicked)
        self.builder.get_object("bt_quit").connect("clicked",self.on_bt_quit_clicked)

        self.builder.get_object("bt_play").connect("clicked", self.on_bt_play_clicked)
        self.builder.get_object("bt_a").connect("clicked", self.on_bt_a_clicked)
        self.builder.get_object("bt_b").connect("clicked", self.on_bt_b_clicked)
        self.builder.get_object("bt_as").connect("clicked", self.on_bt_as_clicked)
        self.builder.get_object("bt_bs").connect("clicked", self.on_bt_bs_clicked)
        self.builder.get_object("bt_time_add").connect("clicked", self.on_bt_time_add_clicked)
        self.builder.get_object("bt_time_sub").connect("clicked", self.on_bt_time_sub_clicked)
        self.builder.get_object("bt_audio_add").connect("clicked", self.on_bt_audio_add_clicked)
        self.builder.get_object("bt_audio_del").connect("clicked", self.on_bt_audio_del_clicked)
        self.builder.get_object("bt_audio_apply").connect("clicked", self.on_bt_audio_apply_clicked)
        self.builder.get_object("bt_audio_split").connect("clicked", self.on_bt_audio_split_clicked)
        self.builder.get_object("bt_audio_merge").connect("clicked", self.on_bt_audio_merge_clicked)
        self.builder.get_object("bt_dst_add").connect("clicked", self.on_bt_dst_add_clicked)
        self.builder.get_object("bt_dst_clean").connect("clicked", self.on_bt_dst_clean_clicked)
        self.builder.get_object("bt_dst_merge").connect("clicked", self.on_bt_merge_clicked)


        self.window.show_all()


    def create_store(self):
        # filename, preview jpg, filepath, big jpg, fileinfo [length,width,height]
        store = gtk.ListStore(str, gtk.gdk.Pixbuf, str, gtk.gdk.Pixbuf, str)
        return store
        
    def get_icon(self,name):
        theme = gtk.icon_theme_get_default()
        return theme.load_icon(name,48,0)

    def on_item_activated(self,widget, item):
        model = widget.get_model()
        path = model[item][COL_PATH]
        self.player.preview(path,"","")

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
        str_len = utils.time_to_string(float(length))
        name = model[item][COL_NAME]
        context = name +":"+str_len
        self.statusbar.push(0,context)

    def on_audio_item_selection_changed(self, widget):
        model = widget.get_model()
        selected = widget.get_selected_items()
        if len(selected) == 0:
            return
        item = selected[0][0]
        name = model[item][COL_NAME]
        info = model[item][COL_INFO]
        infos = info.strip('[]').split(',')
        length = infos[0].strip('\'')
        a = infos[1].strip(' \'')
        b = infos[2].strip(' \'')
        str_len = utils.time_to_string(float(length))
        context = name +":"+str_len
        self.statusbar.push(0,context)
        print "audio file a-b",a, b


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

    def on_bt_dst_add_clicked(self, widget):
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
            self.load_dst_file(fn_widget.get_filename())
        fn_widget.destroy()

    def on_bt_audio_add_clicked(self, widget):
        fn_widget = gtk.FileChooserDialog("Select a Video File",None,gtk.FILE_CHOOSER_ACTION_OPEN,buttons=(gtk.STOCK_CANCEL,gtk.RESPONSE_CANCEL, gtk.STOCK_OPEN,gtk.RESPONSE_OK))
        fn_widget.set_local_only(True)
        fn_filter = gtk.FileFilter()
        fn_filter.set_name("Audio/*")
        fn_filter.add_mime_type("Audio/*")
        fn_widget.add_filter(fn_filter)
        fn_filter_all = gtk.FileFilter()
        fn_filter_all.set_name("all file")
        fn_filter_all.add_pattern("*")
        fn_widget.add_filter(fn_filter_all)
        if fn_widget.run() == gtk.RESPONSE_OK:
            self.load_audio_file(fn_widget.get_filename())
        fn_widget.destroy()

    def on_bt_audio_del_clicked(self, widget):
        model = self.iconview_audio.get_model()
        selected = self.iconview_audio.get_selected_items()
        if len(selected) == 0:
            return
        item = selected[0][0]
        iter = model.get_iter(item)
        model.remove(iter)

    def on_bt_audio_apply_clicked(self, widget):
        model = self.iconview_audio.get_model()
        selected = self.iconview_audio.get_selected_items()
        if len(selected) == 0:
            return
        a_time = self.label_A.get_text()
        b_time = self.label_B.get_text()
        item = selected[0][0]
        info = model[item][COL_INFO]
        infos = info.strip('[]').split(',')
        length = infos[0].strip('\'')
        model[item][COL_INFO] = [length, a_time, b_time]
        #dirIcon = self.get_icon(gtk.STOCK_OPEN)
        model[item][COL_PIXBUF] = self.applyIcon

    def on_bt_audio_split_clicked(self, widget):
        a_model = self.iconview_audio.get_model()
        v_model = self.iconview_src.get_model()
        v_selected = self.iconview_src.get_selected_items()
        if len(v_selected) == 0:
            return
        a_selected = self.iconview_audio.get_selected_items()
        if len(a_selected) == 0:
            return
        v_item = v_selected[0][0]
        video_filename = v_model[v_item][COL_PATH]
        a_item = a_selected[0][0]
        audio_filename = a_model[a_item][COL_PATH]
        info = a_model[a_item][COL_INFO]
        infos = info.strip('[]').split(',')
        a_time = infos[1].strip(' \'')
        b = infos[2].strip(' \'')
        b_time = utils.string_time_sub(b, a_time)
        subffix = utils.get_file_subffix(video_filename)
        outfile = evhome_dir+"outfile_"+str(self.file_num) + subffix
        cmd = "mencoder" + " -ss " + a_time + " -endpos " + b_time + "  -ovc copy -oac mp3lame -audiofile " + audio_filename + " " + video_filename + " -o " + outfile
        print cmd
        self.file_num += 1
        self.statusbar.push(0,cmd)
        self.wait_run(cmd)
        self.statusbar.push(0,"done")
        self.load_dst_file(outfile)


    def on_bt_audio_merge_clicked(self, widget):
        a_model = self.iconview_audio.get_model()
        v_model = self.iconview_src.get_model()
        selected = self.iconview_src.get_selected_items()
        if len(selected) == 0:
            return
        item = selected[0][0]
        video_filename = v_model[item][COL_PATH]
        v_info = v_model[item][COL_INFO]
        v_infos = v_info.strip('[]').split(',')
        v_length = v_infos[0].strip('\'')
        v_end = utils.time_to_string(float(v_length))

        audio_filename = ""
        iter = a_model.get_iter_first()
        time_p = "00:00:00"
        cmd = ""
        num = 0
        subffix = utils.get_file_subffix(video_filename)
        cmd_list =[]
        while ( iter != None ):
            row = a_model.get_path(iter)
            audio_filename = a_model[row][COL_PATH]
            info = a_model[row][COL_INFO]
            infos = info.strip('[]').split(',')
            a_time = infos[1].strip(' \'')
            b = infos[2].strip(' \'')
            if b == "0":
                iter = a_model.iter_next(iter)
                continue
            b_time = utils.string_time_sub(b, a_time)
            if a_time == time_p:
                cmd = "mencoder -ss "+ a_time + " -endpos "+ b_time +" -ovc copy -oac mp3lame -audiofile "+audio_filename + " " + video_filename + " -o /tmp/dumpvideo" + str(num) +subffix
                cmd_list.append(cmd)
                num +=1
                time_p = b_time
            else:
                cmd = "mencoder -ss "+ time_p + " -endpos "+ a_time +" -ovc copy -oac copy " + video_filename + " -o /tmp/dumpvideo" + str(num) +subffix
                cmd_list.append(cmd)
                num +=1
                cmd = "mencoder -ss "+ a_time + " -endpos "+ b_time +" -ovc copy -oac mp3lame -audiofile "+audio_filename + " " + video_filename + " -o /tmp/dumpvideo" + str(num) +subffix
                cmd_list.append(cmd)
                num +=1
                time_p = b_time
                
            iter = a_model.iter_next(iter)
        if time_p != v_end:
                cmd = "mencoder -ss "+ time_p + " -endpos "+ v_end +" -ovc copy -oac copy " + video_filename + " -o /tmp/dumpvideo" + str(num) +subffix
                cmd_list.append(cmd)
                num += 1

        filelist=""
        for i in range(0,num):
            filelist += " /tmp/dumpvideo"+str(i)+subffix
        cmd =" mencoder -ovc lavc -oac mp3lame -o /tmp/dumpvideo.avi "+filelist
        cmd_list.append(cmd)
        num += 1

        for i in range(0,num):
            self.statusbar.push(0,cmd)
            print cmd_list[i]
            self.wait_run(cmd_list[i])
        self.statusbar.push(0,"done")
        self.load_src_file("/tmp/dumpvideo.avi")

    def on_bt_src_del_clicked(self,widget):
        model = self.iconview_src.get_model()
        selected = self.iconview_src.get_selected_items()
        if len(selected) == 0:
            return
        item = selected[0][0]
        iter = model.get_iter(item)
        model.remove(iter)
        #pix = gtk.gdk.pixbuf_new_from_file(app_png)
        #self.preview_image.set_from_pixbuf(pix)


    def on_bt_quit_clicked(self,widget):
        gtk.main_quit()

    def on_bt_dst_clean_clicked(self, widget):
        model = self.iconview_dst.get_model()
        model.clear()

    def on_bt_split_clicked(self, widget):
        model = self.iconview_src.get_model()
        selected = self.iconview_src.get_selected_items()
        if len(selected) == 0:
            return
        item = selected[0][0]
        filename = model[item][COL_PATH]

        a_time = self.label_A.get_text()
        b = self.label_B.get_text()
        b_time = utils.string_time_sub(b,a_time)
        #b_time = utils.time_to_string(self.timeline.getB() - self.timeline.getA())
        subffix = utils.get_file_subffix(filename)
        outfile = evhome_dir + "outfile_" + str(self.file_num) + subffix
        if subffix != ".avi":
            outfile = evhome_dir + "outfile_" + str(self.file_num) + ".avi"
            cmd = "mencoder"+" -ss "+a_time+" -endpos "+b_time+"  -ovc lavc -oac pcm "+filename+" -o "+outfile
        else:
            cmd = "mencoder"+" -ss "+a_time+" -endpos "+b_time+"  -ovc copy -oac copy "+filename+" -o "+outfile
        self.file_num += 1
        print cmd
        print self.file_num
        self.statusbar.push(0,cmd)
        self.wait_run(cmd)
        self.statusbar.push(0,"done")
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
            subffix = utils.get_file_subffix(model[row][COL_PATH])
            info = model[row][COL_INFO]
            infos = info.strip('[]').split(',')
            print infos , infos[1]
            w = infos[1].strip(' \'')
            h = infos[2].strip(' \'')
            if int(w) < size_w :
                size_w = int(w)
                size_h = int(h)
            iter = model.iter_next(iter)
        if filename == "":
            return 0
        pic_size = str(size_w)+":"+str(size_h)
        cmd = "mencoder -ovc lavc -oac mp3lame -idx -vf scale=" + pic_size + " " + filename+" -o /tmp/output"+subffix
        print cmd
        self.statusbar.push(0,cmd)
        self.wait_run(cmd)
        self.statusbar.push(0,"done")

    def on_bt_play_clicked(self, widget):
        model = self.iconview_src.get_model()
        selected = self.iconview_src.get_selected_items()
        if len(selected) == 0:
            return
        item = selected[0][0]
        filename = model[item][COL_PATH]
        a_time = self.label_A.get_text()
        b_time = utils.time_to_string(self.timeline.getB() - self.timeline.getA())
        self.player.preview(filename,a_time,b_time)
        
    def on_bt_a_clicked(self, widget):
        val = self.timeline.get_value()
        self.timeline.setA(float('%.2f'%val))
        a = self.timeline.getA()
        self.label_A.set_text(utils.time_to_string(a))
        self.time_mark = "A"

    def on_bt_b_clicked(self, widget):
        val = self.timeline.get_value()
        self.timeline.setB(float('%.2f'%val))
        b = self.timeline.getB()
        self.label_B.set_text(utils.time_to_string(b))
        self.time_mark = "B"

    def on_bt_as_clicked(self, widget):
        self.timeline.syncA()
        self.time_mark = "A"

    def on_bt_bs_clicked(self, widget):
        self.timeline.syncB()
        self.time_mark = "B"

    def on_bt_time_add_clicked(self, widget):
        if self.time_mark == "A":
            astr = self.label_A.get_text()
            a = utils.string_to_time(astr)
            a = a+1
            self.label_A.set_text(utils.time_to_string(a))
        elif self.time_mark == "B":
            astr = self.label_B.get_text()
            b = utils.string_to_time(astr)
            b = b+1
            self.label_B.set_text(utils.time_to_string(b))
        
    def on_bt_time_sub_clicked(self, widget):
        if self.time_mark == "A":
            astr = self.label_A.get_text()
            a = utils.string_to_time(astr)
            a = a-1
            if a< 0 :
                a = 0
            self.label_A.set_text(utils.time_to_string(a))
        elif self.time_mark == "B":
            astr = self.label_B.get_text()
            b = utils.string_to_time(astr)
            b = b-1
            if b < 0:
                b=0
            self.label_B.set_text(utils.time_to_string(b))

    def on_about_clicked(self, widget):
        about = gtk.AboutDialog()
        about.set_name("Easy Video maker")
        about.set_version("0.01")
        about.set_copyright("Copyright@lerosua 2010")
        about.set_comments("A GUI for mencoder")
        about.set_authors(["郭嘉","lerosua@gmail.com"])
        about.set_license("GPLv2")
        about.set_website("http://www.lerosua.org")
        about.run()
        about.hide()



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
        length,width,height = self.player.get_info(filename)
        tmpicon = gtk.gdk.pixbuf_new_from_file_at_size(jpg,96,96)
        tmpicon_preview = gtk.gdk.pixbuf_new_from_file_at_size(jpg,400,300)
        name = os.path.basename(filename)
        self.store_dst.append([name,tmpicon,filename,tmpicon_preview, [length,width,height]])


    def load_audio_file(self, filename):
        length = self.player.get_length(filename)
        #jpg  = self.get_icon(gtk.STOCK_FILE)
        #jpg =  gtk.gdk.pixbuf_new_from_file_at_size(audio_png,48,48)
        name = os.path.basename(filename)
        # if audio file , the info is length a_time,b_time(a-b is time of video)
        self.store_audio.append([name, self.audioIcon, filename, self.audioIcon, [length,"0","0"]])
        #self.store_audio.append([name,jpg,filename,jpg, [length,"0","0"]])

    def drag_data_src_get(self,widget, context, selection_data, info, timestamp):
        print "drag get"

    def drag_data_src_received(self, widget, context, x, y, selection, targetType, timestamp):
        #filename = selection.data.strip()
        #tmp = unquote(filename.strip('[\']'))
        #self.load_src_file(tmp[7:])

        uri = selection.data.strip('\r\n\x00')
        uri_splitted = uri.split()
        for uri in uri_splitted:
            path = utils.get_file_path_from_dnd_dropped_uri(uri)
            self.load_src_file(path)

    def drag_data_audio_received(self, widget, context, x, y, selection, targetType, timestamp):
        uri = selection.data.strip('\r\n\x00')
        uri_splitted = uri.split()
        for uri in uri_splitted:
            path = utils.get_file_path_from_dnd_dropped_uri(uri)
            self.load_audio_file(path)

    def drag_data_dst_received(self, widget, context, x, y, selection, targetType, timestamp):
        uri = selection.data.strip('\r\n\x00')
        uri_splitted = uri.split()
        for uri in uri_splitted:
            path = utils.get_file_path_from_dnd_dropped_uri(uri)
            self.load_dst_file(path)

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

