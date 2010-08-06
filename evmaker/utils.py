#!/usr/bin/env python
# -*- coding: UTF-8 -*-
import urllib

def get_file_path_from_dnd_dropped_uri(uri):
   # get the path to file
   path = ""
   if uri.startswith('file:\\\\\\'): # windows
       path = uri[8:] # 8 is len('file:///')
   elif uri.startswith('file://'): # nautilus, rox
       path = uri[7:] # 7 is len('file://')
   elif uri.startswith('file:'): # xffm
       path = uri[5:] # 5 is len('file:')

   path = urllib.url2pathname(path) # escape special chars
   path = path.strip('\r\n\x00') # remove \r\n and NULL

   return path

def time_to_string(value):
    """
    Converts the given time in nanoseconds to a human readable string

    Format HH:MM:SS
    """
    mins = value / 60
    sec = value % 60
    hours = mins / 60
    mins = mins % 60
    return "%02d:%02d:%02d" % (hours, mins, sec)

def string_to_time(value):
    """
    Converts the given string HH:MM:SS to time second 
    """
    astr = value.split(':')
    return int(astr[0])*3600 + int(astr[1])*60 + int(astr[2])

def string_time_sub(a_time,b_time):
    """
    return the a_time - b_time. like 00:20:10 - 00:10:19
    """
    a = string_to_time(a_time)
    b = string_to_time(b_time)
    return time_to_string(a-b)
