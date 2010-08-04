#!/usr/bin/env python
# -*- coding: UTF-8 -*-


import os
import gtk
import pygtk
from markscale import MarkScale



window = gtk.Window()
adj = gtk.Adjustment(0.0,0.0,100.0,0.1,1.0,1.0)
timeline = MarkScale(adj)

timeline.setNbFrames(100)
timeline.setA(20)
timeline.setB(30)
timeline.set_update_policy(gtk.UPDATE_CONTINUOUS)
timeline.set_digits(1)
timeline.set_value_pos(gtk.POS_TOP)
timeline.set_draw_value(False)

vbox = gtk.VBox()
vbox.pack_start(timeline,False,False)
button = gtk.Button()
vbox.pack_start(button)
window.add(vbox)
window.set_default_size(400,200)
window.show_all()
gtk.main()


