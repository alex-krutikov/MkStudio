#!/usr/bin/env python

import glob
import os
import cgi
import cgitb
import re

regexp = r"^[a-z0-9]{1,16}$"

cgitb.enable()

print("Content-type: text/html")
print("") 

form = cgi.FieldStorage()

files=[]

if "name" in form:
    name = form["name"].value
    if re.match(regexp, name):
        files = glob.glob(name + "/*.bin")
        files.sort(key=os.path.getmtime, reverse=True)

print('<?xml version="1.0"?>')
print("<versions>")
for f in files:
    f = f.split("/")[-1]
    f = f.split(".")[0]
    print("  <item>" + f + "</item>")
print("</versions>")
