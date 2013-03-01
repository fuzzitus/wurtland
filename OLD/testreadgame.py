#!/usr/bin/python
import os
import sys
sys.path.append(os.getcwd() + '/redemptionweb')
sys.path.append(os.getcwd() + '/servermodules')
#This is a module I made that outputs python as html
from redemptionweb import *
import ciPack as p

F = open('/home4/fuzzitus/public_html/game.fuz', 'rb')
print 'Byte: ' + str(p.ReadByte(F))
print 'String: ' + str(p.ReadString(F))
F.close()

F = open('/home4/fuzzitus/public_html/gameo.fuz', 'rb')
p.WriteString('Does this work?')
F.close()
