#!/usr/bin/python
import os
import sys
sys.path.append(os.getcwd() + '/servermodules')
sys.path.append(os.getcwd() + '/redemptionweb')
from redemptionweb import *
import fz_player as p
import fz_beacon as b
import fz_game as g

#This performs server activity!

#This is the Host address
HOST = 'http://www.fuzzit.us/'

#http://www.fuzzit.us/cgi-bin/CreateGame.py
if len(Pars) == 0:
    print '<meta http-equiv="refresh" content="0; url=' + HOST  + 'adminconsole.py">'

else:
    #http://www.fuzzit.us/cgi-bin/CreateGame.py?servertype=ST&gamename=GN&pw=PW
    SERVERTYPE = Pars['servertype'].value
    GAME_NAME = Pars['gamename'].value
    PW = Pars['pw'].value

    #Checks to see if the the name is already taken
    if os.path.exists(os.getcwd() + '/games/' + g.Format(GAME_NAME) + '.fuz'):
        print t.Text('Game Name Already Taken! :O', dict(size='36', align='center'))
        print '<meta http-equiv="refresh" content="0; url=' + HOST  + 'adminconsole.py?action=createnewgame">'
    else:
        g.Game(GAME_NAME, PW, False, [], [], []).Save()
        print '<meta http-equiv="refresh" content="0; url=' + HOST  + 'adminconsole.py?action=adminmenu&servertype=' + SERVERTYPE + '&gamename=' + GAME_NAME + '&pw=' + PW + '">'
        
