#!/usr/bin/env python
# -*- coding: UTF-8 -*-

import os
import re
import sys

tmp_dir="/tmp/"
preview_jpg_filename = "/tmp/00000001.jpg"

class player():
    def get_length(self,filename):
        cmd='mplayer -identify -frames 0 2>/dev/null '+filename
        out = os.popen(cmd)
        for line in out:
            line = line.strip()
            if re.search("ID_LENGTH",line):
                return line[10:]
        return 0

    def get_info(self,filename):
        cmd='mplayer -identify -frames 0 2>/dev/null '+filename
        out = os.popen(cmd)
        length = 0
        width = 0
        height  = 0
        for line in out:
            line = line.strip()
            if re.search("ID_LENGTH",line):
                length = line[10:]
            if re.search ("ID_VIDEO_WIDTH", line):
                width = line[15:]
            if re.search ("ID_VIDEO_HEIGHT", line):
                height = line[16:]
        return length,width,height

    def get_screenshot(self,filename):
        os.chdir(tmp_dir)
        cmd  = "mplayer -ss 90 -noframedrop -nosound -vo jpeg -frames 1 2>/dev/null "+filename
        #out = os.popen(cmd)
        self.wait_run(cmd)
        return preview_jpg_filename


    def preview(self,filename,a_time,b_time):
        if b_time == "":
            self.run("mplayer","-osdlevel","3",filename)
        else:
            self.run("mplayer","-osdlevel","3","-ss",a_time,"-endpos",b_time,filename)

    def run(self,program, *args):
        pid = os.fork()
        if not pid:
            os.execvp(program,(program,)+args)

    def wait_run(self,cmd):
        pid = os.fork()
        if not pid:
            os.system(cmd)
            sys.exit(0)
        return os.wait()[0]
