#!/usr/bin/env python
# -*- coding: UTF-8 -*-


import os
import gtk
import pygtk
from markscale import MarkScale



window = gtk.Window()
timeline = MarkScale(window)


window.add(timeline)
window.show_all()
gtk.main()


