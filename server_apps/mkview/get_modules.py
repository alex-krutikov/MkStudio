#!/usr/bin/env python

import cgi
import cgitb
import glob
import hashlib
import os


def calc_sha1(filename):
    with open(filename, 'rb') as f:
        m = hashlib.sha1()
        while True:
            data = f.read(64 * 1024)
            if not data:
                break
            m.update(data)
        return m.hexdigest()


cgitb.enable()

print("Content-type: text/html")
print("")

print('<?xml version="1.0"?>')
print('<modules>')

for filename in glob.glob("modules/*.xml"):
    filename = filename.split(os.sep)[-1]
    sha1 = calc_sha1("modules/" + filename)
    print('    <module filename="{}" sha1="{}"/>'.format(filename, sha1))

print("</modules>")
