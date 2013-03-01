#!/usr/bin/python
import os
import sys
sys.path.append(os.getcwd() + '/servermodules')
sys.path.append(os.getcwd() + '/redemptionweb')
from redemptionweb import *

import fz_player as p
import fz_beacon as b
import fz_game as g
import fz_mobileprotocol as mp

import ciPack as p

#This performs server activity!

#This is the Host address
HOST = 'http://www.fuzzit.us/'

#http://www.fuzzit.us/cgi-bin/GlobalServer.py
if len(Pars) == 0:
    GoToPage(HOST)
    
else:
    
    if 'action' not in Pars:
        GoToPage(HOST)
    else:
        ACTION = Pars['action'].value

        #Creates a game or returns back if that name is taken
        #http://www.fuzzit.us/cgi-bin/GlobalServer.py?action=creategame&gamename=GAME&pw=PASSWORD&servertype=static
        if ACTION == 'creategame':
            GAMENAME = Pars['gamename'].value
            PW = Pars['pw'].value
            SERVERTYPE = Pars['servertype'].value

            if g.CheckGameExists(GAMENAME):
                if g.CheckPassword(GAMENAME, PW):
                    g.Game(GAMENAME, PW, False, [], [], [], []).Save()
                    GoToPage(HOST + 'cgi-bin/adminconsole.py?action=adminmenu&servertype=' + SERVERTYPE + '&gamename=' + GAMENAME + '&pw=' + PW)
                else:
                    GoToPage(HOST + 'cgi-bin/adminconsole.py?action=createnewgame')
            else
                GoToPage(HOST + 'cgi-bin/adminconsole.py?action=createnewgame')


        #Logs into a game or returns back if the password is wrong
        #http://www.fuzzit.us/cgi-bin/GlobalServer.py?action=login&gamename=GAME&pw=PASSWORD&servertype=static
        elif ACTION == 'logingame':
            GAMENAME = Pars['gamename'].value
            PW = Pars['pw'].value
            SERVERTYPE = Pars['servertype'].value

            if g.CheckGameExists(GAMENAME):
                if g.CheckPassword(GAMENAME, PW):
                    GoToPage(HOST + 'cgi-bin/adminconsole.py?action=adminmenu&servertype=' + SERVERTYPE + '&gamename=' + GAMENAME + '&pw=' + PW)
                else:
                    GoToPage(HOST + 'cgi-bin/adminconsole.py?action=logintogame')
            else
                GoToPage(HOST + 'cgi-bin/adminconsole.py?action=logintogame')


        #A Mobile Device Requests For A Game
        #http://www.fuzzit.us/cgi-bin/GlobalServer.py?action=mobilegetgame&gamename=GAME&pw=PASSWORD
        elif ACTION == 'mobilegetgame':
            GAMENAME = Pars['gamename'].value
            PW = Pars['pw'].value

            if g.CheckGameExists(GAMENAME):
                if g.CheckPassword(GAMENAME, PW):
                    GAME = g.LoadGame(GAMENAME)
                    ################################################DO STUFF - INCOMPLETE
                else:
                    mp.WrongPassword()
            else:
                mp.WrongGameName()

