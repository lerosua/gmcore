#!/usr/bin/env python
# -*- coding: UTF-8 -*-

import os
import re

tmp_dir="/tmp/"
preview_jpg_filename = "/tmp/00000001.jpg"

class player():
    def get_length(self,filename):
        cmd='mplayer -indentify -frames 0 2>/dev/null '+filename
        out = os.popen(cmd)
        for line in out:
            line = line.strip()
            if re.search("ID_LENGTH",line):
                return line[10:]
        return 0

    def get_screenshot(self,filename):
        os.chdir(tmp_dir)
        cmd  = "mplayer -ss 90 -noframedrop -nosound -vo jpeg -frames 1 2>/dev/null "+filename
        out = os.popen(cmd)
        return preview_jpg_filename


    def preview(self,*args):
        self.run("mplayer","-osdlevel","3",*args)

    def run(self,program, *args):
        pid = os.fork()
        if not pid:
            os.execvp(program,(program,)+args)
