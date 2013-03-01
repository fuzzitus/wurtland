#!/usr/bin/python
import os
import sys
sys.path.append(os.getcwd() + '/redemptionweb')
sys.path.append(os.getcwd() + '/servermodules')
#This is a module I made that outputs python as html
from redemptionweb import *

#This is the Host address
HOST = 'http://www.fuzzit.us/'

#Properties for the Main Div
#I have them here in case any of the pages need to altar them
DIV_PROPS = dict(id='main',
                 align='center',
                 style=a.toCssStyle(dict(position='relative',
                                         top='35%')))
DIV_OBJS = []

#The Starting page: http://www.fuzzit.us/cgi-bin/adminconsole.py
if len(Pars) == 0:
    LOGO = i.Image('logoS', 'http://www.fuzzit.us/graphics/beacon_logo.png')
    LOG_IN_BUTTON = l.Link(HOST + 'cgi-bin/adminconsole.py?action=logintogame', [b.Button('logintogame', 'Log In To Existing Game')])
    NEW_GAME_BUTTON = l.Link(HOST + 'cgi-bin/adminconsole.py?action=createnewgame', [b.Button('createnewgame', 'Create New Game')])

    DIV_PROPS = dict(id='main',
                 align='center',
                 style=a.toCssStyle(dict(position='relative',
                                         top='20%')))
    
    DIV_OBJS = [LOGO, t.br, t.T('<font size=5>'),
                LOG_IN_BUTTON, t.br,
                NEW_GAME_BUTTON, t.T('</font>')]
    
else:
    #Checks for 'action='
    if 'action' not in Pars:
        #Returns to the mainpage
        print '<meta http-equiv="refresh" content="0; url=' + HOST + 'cgi-bin/adminconsole.py">'
    else:
        ACTION = Pars['action'].value

        #Log In Screen
        #http://www.fuzzit.us/cgi-bin/adminconsole.py?action=logintogame
        if ACTION == 'logintogame':
            
            TITLE = t.Text('Login To A Game', dict(size='36'))

            SERVER = f.TextField('servertype', 'Server:')
            GAME_NAME = f.TextField('gamename', 'Game Name:')
            PW = f.TextField('pw', 'Password:')

            LOGIN_FUNC = j.Func('LoginGame()', [], LinkTo(HOST + 'cgi-bin/GlobalServer.py?action=logingame&servertype= " + ' + ElementAtt('servertype') + ' + "&gamename=" + ' + ElementAtt('gamename') + ' + "&pw=" + ' + ElementAtt('pw')))
            
            CREATE = b.Button('login', 'Login', dict(onclick='LoginGame()'))
            
            DIV_OBJS = [TITLE, t.br,
                        SERVER, t.br,
                        GAME_NAME, t.br,
                        PW, t.br, LOGIN_FUNC,
                        CREATE]
            
        #Log In Screen
        #http://www.fuzzit.us/cgi-bin/adminconsole.py?action=logintogame
        elif ACTION == 'createnewgame':
            
            TITLE = t.Text('Create New Game', dict(size='36'))

            SERVER = f.TextField('servertype', 'Server:')
            GAME_NAME = f.TextField('gamename', 'Game Name:')
            PW = f.TextField('pw', 'Password:')

            CREATE_FUNC = j.Func('CreateGame()', [], LinkTo(HOST + 'cgi-bin/GlobalServer.py?action=creategame&servertype=" + ' + ElementAtt('servertype') + ' + "&gamename=" + ' + ElementAtt('gamename') + ' + "&pw=" + ' + ElementAtt('pw')))
            
            CREATE = b.Button('create', 'Create New Game', dict(onclick='CreateGame()'))
            
            DIV_OBJS = [TITLE, t.br,
                        SERVER, t.br,
                        GAME_NAME, t.br,
                        PW, t.br, CREATE_FUNC,
                        CREATE]
            
        #Admin Menu
        #http://www.fuzzit.us/cgi-bin/adminconsole.py?action=adminmenu
        elif ACTION == 'adminmenu':
            GAMENAME = Pars['gamename'].value
            PW = Pars['pw'].value

            TITLE = t.Text('Menu', dict(size='36'))

            BEACON_FUNC = j.Func('GoToBeacon()', [], LinkTo(HOST + 'cgi-bin/adminconsole.py?action=beaconmenu&gamename=" + ' + ElementAtt('gamename') + ' + "&pw=" + ' + ElementAtt('pw')))
            PLAYER_FUNC = j.Func('GoToPlayer()', [], LinkTo(HOST + 'cgi-bin/adminconsole.py?action=playertraits&gamename=" + ' + ElementAtt('gamename') + ' + "&pw=" + ' + ElementAtt('pw')))
            TRAITS_FUNC = j.Func('GoToTraits()', [], LinkTo(HOST + 'cgi-bin/adminconsole.py?action=traits&gamename=" + ' + ElementAtt('gamename') + ' + "&pw=" + ' + ElementAtt('pw')))
            INS_FUNC = j.Func('GoToIns()', [], LinkTo(HOST + 'cgi-bin/adminconsole.py?action=instructions&gamename=" + ' + ElementAtt('gamename') + ' + "&pw=" + ' + ElementAtt('pw')))
            EXIT_FUNC = j.Func('GoToExit()', [], LinkTo(HOST + 'cgi-bin/adminconsole.py?'))

            BEACON_BUTTON = b.Button('arbeacons', 'Add/Remove Beacons', dict(onclick='GoToBeacon()'))
            PLAYER_BUTTON = b.Button('playertraits', 'Manually Assign Player Traits', dict(onclick='GoToPlayer()'))
            TRAITS_BUTTON = b.Button('traits', 'Create Custom Traits', dict(onclick='GoToTraits()'))
            INS_BUTTON = b.Button('ins', 'Configure Instructions', dict(onclick='GoToIns()'))
            EXIT_BUTTON = b.Button('exit', 'Exit', dict(onclick='GoToExit()'))

            DIV_OBJS = [TITLE, t.br,
                        BEACON_FUNC, t.T('Beacons: '), BEACON_BUTTON, t.br,
                        PLAYER_FUNC, t.T('Players: '), PLAYER_BUTTON, t.br,
                        TRAITS_FUNC, t.T('Traits: '), TRAITS_BUTTON, t.br,
                        INS_FUNC, t.T('Instruction: '), INS_BUTTON, t.p,
                        EXIT_FUNC, EXIT_BUTTON, t.br]
            
        #Beacon Menu
        #http://www.fuzzit.us/cgi-bin/adminconsole.py?action=beaconmenu
        elif ACTION == 'adminmenu':
            import fz_game as g
            GAMENAME = Pars['gamename'].value
            PW = Pars['pw'].value

            GAME = g.Load(GAMENAME)

            TITLE = t.Text('Add/Remove Beacons', dict(size='36'))

            BEACON_LIST = f.Dropdown('beaconlist')
            for EB in GAME.Beacons:
                BEACON_LIST.Values.append([EB.Id, EB.Id])
            BEACON_LIST.Values.append(['create_new_beacon', 'New Beacon'])

            BEACON_NAME = f.TextField('b__beacon__name', 'Name: ')

            UPDATE_FUNC = j.Func('UpdateBeacon()', [], LinkTo(HOST + 'cgi-bin/GlobalServer.py?action=updatebeacon&newname=" + ' + ElementAtt('b__beacon__name') + ' + "&gamename=" + ' + ElementAtt('gamename') + ' + "&pw=" + ' + ElementAtt('pw')))
            DELETE_FUNC = j.Func('DeleteBeacon()', [], LinkTo(HOST + 'cgi-bin/GlobalServer.py?action=deletebeacon&newname=" + ' + ElementAtt('b__beacon__name') + ' + "&gamename=" + ' + ElementAtt('gamename') + ' + "&pw=" + ' + ElementAtt('pw')))
            
            UPDATE_BUTTON = b.Button('update', 'UpdateBeacon()', dict(onclick='UpdateBeacon()'))
            DELETE_BUTTON = b.Button('delete', 'DeleteBeacon()', dict(onclick='DeleteBeacon()'))
            MENU_BUTTON = l.Link(HOST + 'cgi-bin/adminconsole.py?action=adminmenu&gamename=' + GAMENAME + '&pw=' + PW, [b.Button('menu', 'Menu')])

            DIV_OBJS = [TITLE, BEACON_LIST
            

#EXECUTE!
div.Div(DIV_PROPS, DIV_OBJS)()
