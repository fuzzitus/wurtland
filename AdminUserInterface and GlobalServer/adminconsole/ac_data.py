#!/usr/bin/python

#Import Global Modules
import os
import sys
sys.path.append(os.getcwd() + '/redemptionweb')
sys.path.append(os.getcwd() + '/servermodules')
from redemptionweb import *

#Strings
HOST = 'http://www.fuzzit.us'
GFX_DIR = HOST + '/graphics'
HOST_ACT = HOST + '/cgi-bin/adminConsole.py?action='
SERVER = HOST + '/cgi-bin/GlobalServer.py'
SERVER_ACT = SERVER + '?action='
