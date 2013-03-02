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
    LOGO = i.Image('logoS', 'http://www.fuzzit.us/graphics/beacon_logo_lite.png')
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

            LOGIN_FUNC = j.Func('LoginGame', [], j.LinkTo('"' + HOST + 'cgi-bin/GlobalServer.py?action=logingame&servertype= " + ' + j.ElementAtt('servertype') + ' + "&gamename=" + ' + j.ElementAtt('gamename') + ' + "&pw=" + ' + j.ElementAtt('pw') + ';'))
            
            CREATE = b.Button('login', 'Login', dict(onclick='javascript:LoginGame()'))
            
            DIV_OBJS = [TITLE, t.br,
                        SERVER, t.br,
                        GAME_NAME, t.br,
                        PW, t.br, LOGIN_FUNC,
                        CREATE]
            
        #Create New Game Screen
        #http://www.fuzzit.us/cgi-bin/adminconsole.py?action=createnewgame
        elif ACTION == 'createnewgame':
            
            TITLE = t.Text('Create New Game', dict(size='36'))

            SERVER = f.TextField('servertype', 'Server:')
            GAME_NAME = f.TextField('gamename', 'Game Name:')
            PW = f.TextField('pw', 'Password:')

            CREATE_FUNC = j.Func('CreateGame', [], j.LinkTo('"' + HOST + 'cgi-bin/GlobalServer.py?action=creategame&servertype=" + ' + j.ElementAtt('servertype') + ' + "&gamename=" + ' + j.ElementAtt('gamename') + ' + "&pw=" + ' + j.ElementAtt('pw') + ';'))
            
            CREATE = b.Button('create', 'Create New Game', dict(onclick='javascript:CreateGame()'))
            
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

            BEACON_BUTTON = l.Link(HOST + 'cgi-bin/adminconsole.py?action=beaconmenu&gamename=' + GAMENAME + '&pw=' + PW, [b.Button('arbeacons', 'Add/Remove Beacons')])
            PLAYER_BUTTON = l.Link(HOST + 'cgi-bin/adminconsole.py?action=playertraits&gamename=' + GAMENAME + '&pw=' + PW, [b.Button('playertraits', 'Manually Assign Player Traits')])
            TRAITS_BUTTON = l.Link(HOST + 'cgi-bin/adminconsole.py?action=traits&gamename=' + GAMENAME + '&pw=' + PW, [b.Button('traits', 'Create Custom Traits')])
            INS_BUTTON = l.Link(HOST + 'cgi-bin/adminconsole.py?action=instructions&gamename=' + GAMENAME + '&pw=' + PW, [b.Button('ins', 'Configure Instructions')])
            EXIT_BUTTON = l.Link(HOST + 'cgi-bin/adminconsole.py?' + GAMENAME + '&pw=' + PW, [b.Button('exitb', 'Exit')])
    
            DIV_OBJS = [TITLE, t.br,
                        BEACON_BUTTON, t.br,
                        PLAYER_BUTTON, t.br,
                        TRAITS_BUTTON, t.br,
                        INS_BUTTON, t.p,
                        EXIT_BUTTON, t.br]
            
        #Beacon Menu
        #http://www.fuzzit.us/cgi-bin/adminconsole.py?action=beaconmenu
        elif ACTION == 'beaconmenu':
            import fz_game as g
            
            GAMENAME = Pars['gamename'].value
            PW = Pars['pw'].value

            GAME = g.LoadGame(GAMENAME)

            TITLE = t.Text('Add/Remove Beacons', dict(size='36'))

            LOAD_FUNC = j.Func("LoadUp", [], "document.getElementById('beaconlist').value = 'create_new_beacon';")

            DROP_FUNC = t.T("<script language='javascript'>function Conv()\n~~t{\n~~t~~tif (document.getElementById('beaconlist').value == 'create_new_beacon')\n~~t~~t~~t{\n~~t~~t~~t~~tdocument.getElementById('b__beacon__name').value = ''\n~~t~~t~~t}\n~~t~~telse\n~~t~~t{\n~~t~~t~~tdocument.getElementById('b__beacon__name').value = document.getElementById('beaconlist').value\n~~t~~t}\n~~t}\n</script>".replace("~~t", chr(9)))

            BEACON_LIST = f.Dropdown('beaconlist', [], dict(onchange="javascript:Conv()"))
            for EB in GAME.Beacons:
                BEACON_LIST.Values.append([EB.Name, EB.Name])
            BEACON_LIST.Values.append(['create_new_beacon', '-- New Beacon --'])

            BEACON_NAME = f.TextField('b__beacon__name', 'Name: ')

            UPDATE_FUNC = j.Func('UpdateBeacon', [], j.LinkTo('"' + HOST + 'cgi-bin/GlobalServer.py?action=updatebeacon&gamename=' + GAMENAME + '&pw=' + PW + '&newname=" + ' + j.ElementAtt('b__beacon__name') + ' + "&beacon=" + ' + j.ElementAtt('beaconlist') + ';'))
            DELETE_FUNC = j.Func('DeleteBeacon', [], j.LinkTo('"' + HOST + 'cgi-bin/GlobalServer.py?action=deletebeacon&gamename=' + GAMENAME + '&pw=' + PW + '&beacon=" + ' + j.ElementAtt('beaconlist') + ';'))

            UPDATE_BUTTON = b.Button('update', 'UpdateBeacon', dict(onclick='javascript:UpdateBeacon()'))
            DELETE_BUTTON = b.Button('delete', 'DeleteBeacon', dict(onclick='javascript:DeleteBeacon()'))
            MENU_BUTTON = l.Link(HOST + 'cgi-bin/adminconsole.py?action=adminmenu&gamename=' + GAMENAME + '&pw=' + PW, [b.Button('menu', 'Menu')])

            DIV_OBJS = [TITLE, t.br, LOAD_FUNC, DROP_FUNC,
                        BEACON_LIST, t.br,
                        BEACON_NAME, t.p, UPDATE_FUNC, DELETE_FUNC,
                        UPDATE_BUTTON, t.br,
                        DELETE_BUTTON, t.p,
                        MENU_BUTTON, t.T('<body onload="javascript:LoadUp()"></body>')]
            
        #Player Traits Menu
        #http://www.fuzzit.us/cgi-bin/adminconsole.py?action=playertraits
        elif ACTION == 'playertraits':
            pass
            
        #Traits Menu
        #http://www.fuzzit.us/cgi-bin/adminconsole.py?action=traits
        elif ACTION == 'traits':
            GAMENAME = Pars['gamename'].value
            PW = Pars['pw'].value

            import fz_game as g
            import fz_player as pl

            LOAD_FUNC = j.Func("LoadUp", [], "document.getElementById('traitlist').value = 'blank_trait_value';")
            DROP_FUNC = t.T("<script language='javascript'>function Conv()\n~~t{\n~~t~~tif (document.getElementById('traitlist').value == 'blank_trait_value')\n~~t~~t~~t{\n~~t~~t~~t~~tdocument.getElementById('t__trait__name').value = ''\n~~t~~t~~t}\n~~t~~telse\n~~t~~t{\n~~t~~t~~tdocument.getElementById('t__trait__name').value = document.getElementById('traitlist').value\n~~t~~t}\n~~t}\n</script>".replace("~~t", chr(9))) 

            GAME = g.LoadGame(GAMENAME)

            PLAYERS = []
            for EP in GAME.Players:
                if pl.CheckPlayer(EP):
                    PLAYERS.append(pl.LoadPlayer(EP))
            TRAITS = []
            for EP in PLAYERS:
                for ET in EP.Traits:
                    if ET not in TRAITS:
                        TRAITS.append(ET)

            TITLE = t.Text('Add/Remove Traits', dict(size='36'))

            TRAIT_LIST = f.Dropdown('traitlist', [], dict(onchange="javascript:Conv()", width="100"))
            for ET in TRAITS:
                TRAIT_LIST.Values.append([ET, ET])
            TRAIT_LIST.Values.append(['blank_trait_value', '-- New Trait --'])

            NEW_TRAIT = f.TextField('t__trait__name', 'Trait Name:')

            UPDATE_FUNC = j.Func('UpdateTrait', [], j.LinkTo('"' + HOST + 'cgi-bin/GlobalServer.py?action=updatetrait&gamename=' + GAMENAME + '&pw=' + PW + '&newname=" + ' + j.ElementAtt('t__trait__name') + ' + "&trait=" + ' + j.ElementAtt('traitlist') + ';'))
            DELETE_FUNC = j.Func('DeleteTrait', [], j.LinkTo('"' + HOST + 'cgi-bin/GlobalServer.py?action=deletetrait&gamename=' + GAMENAME + '&pw=' + PW + '&trait=" + ' + j.ElementAtt('traitlist') + ';'))

            UPDATE_BUTTON = b.Button('update', 'UpdateTrait', dict(onclick='javascript:UpdateTrait()'))
            DELETE_BUTTON = b.Button('delete', 'DeleteTrait', dict(onclick='javascript:DeleteTrait()'))
            MENU_BUTTON = l.Link(HOST + 'cgi-bin/adminconsole.py?action=adminmenu&gamename=' + GAMENAME + '&pw=' + PW, [b.Button('menu', 'Menu')])

            DIV_OBJS = [TITLE, t.br, LOAD_FUNC, DROP_FUNC,
                        TRAIT_LIST, t.br,
                        NEW_TRAIT, t.p, UPDATE_FUNC, DELETE_FUNC,
                        UPDATE_BUTTON, t.br,
                        DELETE_BUTTON, t.p,
                        MENU_BUTTON, t.T('<body onload="javascript:LoadUp()"></body>')]
        
#EXECUTE!
div.Div(DIV_PROPS, DIV_OBJS)()
