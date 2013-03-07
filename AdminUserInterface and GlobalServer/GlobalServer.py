#!/usr/bin/python
import os
import sys
sys.path.append(os.getcwd() + '/servermodules')
sys.path.append(os.getcwd() + '/redemptionweb')
from redemptionweb import *

import fz_player as P
import fz_beacon as b
import fz_game as g
import fz_mobileprotocol as mp
import fz_instruction as i
import random as R

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

            if g.CheckGameExists(GAMENAME):
                GoToPage(HOST + 'cgi-bin/adminconsole.py?action=createnewgame')
            else:
                g.Game(GAMENAME, PW, False, [], [], [], []).Save()
                GoToPage(HOST + 'cgi-bin/adminconsole.py?action=adminmenu&servertype=' + SERVERTYPE + '&gamename=' + GAMENAME + '&pw=' + PW)
                


        #Logs into a game or returns back if the password is wrong
        #http://www.fuzzit.us/cgi-bin/GlobalServer.py?action=login&gamename=GAME&pw=PASSWORD&servertype=static
        elif ACTION == 'logintogame':
            GAMENAME = Pars['gamename'].value
            PW = Pars['pw'].value

            if g.CheckGameExists(GAMENAME):
                if g.CheckPassword(GAMENAME, PW):
                    GoToPage(HOST + 'cgi-bin/adminconsole.py?action=adminmenu&servertype=' + SERVERTYPE + '&gamename=' + GAMENAME + '&pw=' + PW)
                else:
                    GoToPage(HOST + 'cgi-bin/adminconsole.py?action=logintogame')
            else:
                GoToPage(HOST + 'cgi-bin/adminconsole.py?action=logintogame')

        #Changes the beacon's name
        #http://www.fuzzit.us/cgi-bin/GlobalServer.py?action=updatebeacon
        elif ACTION == 'updatebeacon':
            GAMENAME = Pars['gamename'].value
            PW = Pars['pw'].value
            BEACONNAME = Pars['beacon'].value
            NEWNAME = Pars['newname'].value

            if g.CheckGameExists(GAMENAME):
                if g.CheckPassword(GAMENAME, PW):

                    GAME = g.LoadGame(GAMENAME)
                    if BEACONNAME == 'create_new_beacon':
                        GAME.Beacons.append(b.Beacon(NEWNAME, 0.0, 0.0, [], False, ''))
                        GAME.Save()
                    else:
                        BEACON = GAME.GetBeacon(BEACONNAME)
                        BEACON.Name = NEWNAME
                        GAME.Save()
                    
                    GoToPage(HOST + 'cgi-bin/adminconsole.py?action=beaconmenu&gamename=' + GAMENAME + '&pw=' + PW)
                else:
                    GoToPage(HOST + 'cgi-bin/adminconsole.py')
            else:
                GoToPage(HOST + 'cgi-bin/adminconsole.py')

        #Delete's the beacon
        #http://www.fuzzit.us/cgi-bin/GlobalServer.py?action=deletebeacon
        elif ACTION == 'deletebeacon':
            GAMENAME = Pars['gamename'].value
            PW = Pars['pw'].value
            BEACONNAME = Pars['beacon'].value

            if g.CheckGameExists(GAMENAME):
                if g.CheckPassword(GAMENAME, PW):
                    GAME = g.LoadGame(GAMENAME)
                    for EB in GAME.Beacons:
                         if EB.Name == BEACONNAME:
                             GAME.Beacons.remove(EB)
                             break
                    GAME.Save()
                    GoToPage(HOST + 'cgi-bin/adminconsole.py?action=beaconmenu&gamename=' + GAMENAME + '&pw=' + PW)
                else:
                    GoToPage(HOST + 'cgi-bin/adminconsole.py')
            else:
                GoToPage(HOST + 'cgi-bin/adminconsole.py')

        #Updates Player Trait
        #http://www.fuzzit.us/cgi-bin/GlobalServer.py?action=updateplayertrait
        elif ACTION == 'updateplayertrait':
            GAMENAME = Pars['gamename'].value
            PW = Pars['pw'].value
            PL = Pars['player'].value

            if PL == '':
                GoToPage(HOST + 'cgi-bin/adminconsole.py?action=playertraits&gamename=' + GAMENAME + '&pw=' + PW)

            if g.CheckGameExists(GAMENAME):
                if g.CheckPassword(GAMENAME, PW):
                    
                    GAME = g.LoadGame(GAMENAME)
                    PLAYER = P.LoadPlayer(PL)
                    PLAYER.Traits = []
                    
                    for ET in Pars:
                        if ET.startswith('cb_fuz_'):
                            PLAYER.Traits.append(ET.replace('cb_fuz_', ''))

                    PLAYER.Save()
                    GAME.Save()
                    
                    GoToPage(HOST + 'cgi-bin/adminconsole.py?action=playertraits&gamename=' + GAMENAME + '&pw=' + PW + '&player=' + PL)
                else:
                    GoToPage(HOST + 'cgi-bin/adminconsole.py')
            else:
                GoToPage(HOST + 'cgi-bin/adminconsole.py')
            

        #Updates a trait
        #http://www.fuzzit.us/cgi-bin/GlobalServer.py?action=updatetrait
        elif ACTION == 'updatetrait':
            GAMENAME = Pars['gamename'].value
            PW = Pars['pw'].value
            TRAIT = Pars['trait'].value
            if g.CheckGameExists(GAMENAME):
                if g.CheckPassword(GAMENAME, PW):
                    GAME = g.LoadGame(GAMENAME)
                    GAME.Traits.append(str(TRAIT))
                    GAME.Save()
                    GoToPage(HOST + 'cgi-bin/adminconsole.py?action=traits&gamename=' + GAMENAME + '&pw=' + PW)
                else:
                    GoToPage(HOST + 'cgi-bin/adminconsole.py')
            else:
                GoToPage(HOST + 'cgi-bin/adminconsole.py')

        #Delete's the trait
        #http://www.fuzzit.us/cgi-bin/GlobalServer.py?action=deletetrait
        elif ACTION == 'deletetrait':
            GAMENAME = Pars['gamename'].value
            PW = Pars['pw'].value
            TRAIT = Pars['trait'].value

            if g.CheckGameExists(GAMENAME):
                if g.CheckPassword(GAMENAME, PW):
                    GAME = g.LoadGame(GAMENAME)
                    if TRAIT in GAME.Traits:
                        GAME.Traits.remove(TRAIT)
                        
                    GAME.Save()
                    GoToPage(HOST + 'cgi-bin/adminconsole.py?action=traits&gamename=' + GAMENAME + '&pw=' + PW)
                else:
                    GoToPage(HOST + 'cgi-bin/adminconsole.py')
            else:
                GoToPage(HOST + 'cgi-bin/adminconsole.py')

        #Updates instruction
        #http://www.fuzzit.us/cgi-bin/GlobalServer.py?action=updateinstruction
        elif ACTION == 'updateinstruction':
            GAMENAME = Pars['gamename'].value
            PW = Pars['pw'].value
            I_NAME = Pars['i__ins__name'].value
            B_NAME = Pars['blist'].value

            T_LIST = []
            for EV in Pars:
                if EV.startswith('st_fuz_'):
                    T_LIST.append(EV.replace('st_fuz_', ''))
                    
            if g.CheckGameExists(GAMENAME):
                if g.CheckPassword(GAMENAME, PW):
                    GAME = g.LoadGame(GAMENAME)
                    I_ID = str(R.randint(0, 9999999999))
                    GAME.Instructions.append(i.Instruction(I_ID, T_LIST, I_NAME))
                    BEACON = GAME.GetBeacon(B_NAME)
                    BEACON.Instructions.append(I_ID)
                    GAME.Save()
                    GoToPage(HOST + 'cgi-bin/adminconsole.py?action=instructions&gamename=' + GAMENAME + '&pw=' + PW)
                else:
                    GoToPage(HOST + 'cgi-bin/adminconsole.py')
            else:
                GoToPage(HOST + 'cgi-bin/adminconsole.py')
			


        #A Mobile Device Requests The List Of Games
        #http://www.fuzzit.us/cgi-bin/GlobalServer.py?action=mobilegetgamelist
        elif ACTION == 'mobilegetgamelist':
            mp.SendGameList()

        #A Mobile Device Requests The List Of Beacons
        #http://www.fuzzit.us/cgi-bin/GlobalServer.py?action=mobilegetgamelist
        elif ACTION == 'mobilegetbeaconlist':
            GAME = Pars['game'].value
            mp.SendBeaconList(GAME)

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

