#!/usr/bin/env python
# -*- coding: UTF-8 -*-

import gtk

class MarkScale(gtk.HScale):

    def __init__(self, adj):
        gtk.HScale.__init__(self,adj)
        self.adj = adj

        self.set_events(gtk.gdk.EXPOSURE_MASK)
        self.markA=0
        self.markB=100
        self.nbFrames=100

        self.connect("expose-event",self.expose)
        self.connect("realize", self.realize)


    def setNbFrames(self,total):
        self.nbFrames = total
        #self.adj.set_upper(total)

    def setA(self, a):
        self.markA=a
        event = gtk.gdk.Event(gtk.gdk.EXPOSE)
        self.emit("expose-event", event)

    def setB(self, b):
        self.markB=b
        if self.markB < self.markA:
            self.markB=self.markA
            self.markA=0
        event = gtk.gdk.Event(gtk.gdk.EXPOSE)
        self.emit("expose-event", event)

    def realize(self, widget):
        widget.set_size_request(widget.allocation.width,widget.allocation.height+10)

    def expose(self, widget, event):
        if self.nbFrames > 1 :
            gc = widget.window.new_gc()
            width = self.allocation.width
            start = self.allocation.x + (width-1)*(self.markA*1.0/(self.nbFrames-1))
            end   = self.allocation.x + (width-1)*(self.markB*1.0/(self.nbFrames-1))
            top   = self.allocation.y + 1;
            bottom = self.allocation.y + self.allocation.height - 2


            widget.window.draw_line(gc, start, top,start+4,top)
            widget.window.draw_line(gc, start, top,start,  top+4)
            widget.window.draw_line(gc, start, bottom, start+4, bottom)
            widget.window.draw_line(gc, start, bottom-4, start, bottom)

            widget.window.draw_line(gc, end, top,end-4,top)
            widget.window.draw_line(gc, end, top,end,  top+4)
            widget.window.draw_line(gc, end, bottom, end-4, bottom)
            widget.window.draw_line(gc, end, bottom-4, end, bottom)

        return False
                
