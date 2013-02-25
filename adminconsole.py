#!/usr/bin/python
import os
import sys
sys.path.append(os.getcwd() + '/redemptionweb')
#This is a module I made that outputs python as html
from redemptionweb import *

#This is the Host address
HOST = 'http://www.fuzzit.us/'

#This is a frame for all of the pages
MAIN_DIV = div.Div(dict(id='main',
                        align='center',
                        style=a.toCssStyle(dict(position='relative',
                                                top='40%'))), [])

#Depending on the page, this will be whatever Objects show up
OBJS = []

#The Starting page: http://www.fuzzit.us/cgi-bin/adminconsole.py
if len(Pars) == 0:
    TITLE = t.Text('iOS Beacon Demo', dict(size='36'))
    LOG_IN_BUTTON = l.Link(HOST + 'cgi-bin/adminconsole.py?action=logintogame', [b.Button('logintogame', 'Log In To Existing Game')])
    NEW_GAME_BUTTON = l.Link(HOST + 'cgi-bin/adminconsole.py?action=createnewgame', [b.Button('createnewgame', 'Create New Game')])
    
    OBJS = [TITLE,
            t.br,
            LOG_IN_BUTTON,
            t.br,
            NEW_GAME_BUTTON]

else:
    #http://www.fuzzit.us/cgi-bin/adminconsole.py?action=ACTION
    ACTION = Pars['action'].value



    #Log In To Game Page: http://www.fuzzit.us/cgi-bin/adminconsole.py?action=logintogame
    if ACTION == 'logintogame':

        #The form for logging into a game

        TITLE = t.Text('Login To A Game', dict(size='36'))

        SERVER = f.TextField('servertype', 'Server:')
        GAME_NAME = f.TextField('gamename', 'Game Name:')
        PW = f.TextField('pw', 'Password:')
        
        CREATE = f.SubmitButton('Login')
        
        NEW_GAME_FORM = f.Form(dict(action='login'), [TITLE, t.br,
                                                      SERVER, t.br,
                                                      GAME_NAME, t.br,
                                                      PW, t.br,
                                                      CREATE])
        OBJS = [NEW_GAME_FORM]



    #Create A New Game Page: http://www.fuzzit.us/cgi-bin/adminconsole.py?action=createnewgame
    elif ACTION == 'createnewgame':

        #The form for creating a game

        TITLE = t.Text('Create New game', dict(size='36'))

        SERVER = f.TextField('servertype', 'Server: ')
        GAME_NAME = f.TextField('gamename', 'Game Name:')
        PW = f.TextField('pw', 'Password: ')
        
        CREATE = f.SubmitButton('Create Game')
        
        NEW_GAME_FORM = f.Form(dict(action='creategame'), [TITLE, t.br,
                                                           SERVER, t.br,
                                                           GAME_NAME, t.br,
                                                           PW, t.br,
                                                           CREATE])
        OBJS = [NEW_GAME_FORM]

    #Menu
    elif ACTION == 'adminmenu':
        SERVERTYPE = Pars['servertype'].value
        GAMENAME = Pars['gamename'].value
        PW = Pars['pw'].value

        TITLE = t.Text('Menu', dict(size='36'))
        
        AR_BEACONS = l.Link(HOST + 'cgi-bin/adminconsole.py?action=arbeacons&gamename=' + GAMENAME + '&pw=' + PW, [b.Button('arbeacon', 'Add/Remove Beacon')])
        CONF_BEACONS = l.Link(HOST + 'cgi-bin/adminconsole.py?action=confbeacons&gamename=' + GAMENAME + '&pw=' + PW, [b.Button('confbeacon', 'Configure Beacons')])
        ASSIGN_TRAITS = l.Link(HOST + 'cgi-bin/adminconsole.py?action=manuallyassigntraits&gamename=' + GAMENAME + '&pw=' + PW, [b.Button('assigntraits', 'Manually Assign Traits')])
        CUSTOM_TRAITS = l.Link(HOST + 'cgi-bin/adminconsole.py?action=customtraits&gamename=' + GAMENAME + '&pw=' + PW, [b.Button('customtraits', 'Create Custom Traits')])

        EXIT_BUTTON = l.Link(HOST + 'cgi-bin/adminconsole.py', [b.Button('exitbutton', 'Exit')])

        OBJS = [TITLE, t.br,
                t.T('New Beacon: '), AR_BEACONS, t.br,
                t.T('Configure: '), CONF_BEACONS, t.br,
                t.T('Players: '), ASSIGN_TRAITS, t.br,
                t.T('Traits: '), CUSTOM_TRAITS, t.br,
                EXIT_BUTTON]

        
                

#Create HTML!
MAIN_DIV.Objs = OBJS
MAIN_DIV()
