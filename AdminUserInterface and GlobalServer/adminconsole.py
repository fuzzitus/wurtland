#!/usr/bin/python
#Import Global Modules
import os
import sys
sys.path.append(os.getcwd() + '/redemptionweb')
sys.path.append(os.getcwd() + '/servermodules')
sys.path.append(os.getcwd() + '/adminconsole')
from redemptionweb import *

#Import Admin Modules
import ac_data as data
import ac_page as page
import ac_objs as objs

#Checking Parameters
if len(Pars) == 0:
    GoToPage(data.HOST_ACT + 'startpage')
else:
    if 'action' not in Pars:
        GoToPage(data.HOST_ACT + 'startpage')
    else:
        ACTION = Pars['action'].value
        
        #PAGES

        #Start Page
        if ACTION == 'startpage':
            page.StartPage('beacon_logo_lite.png',
                           'Log Into Existing Game',
                           'Create New Game')()

        elif ACTION == 'logintogame':
            page.LoginPage('Login To A Game',
                           'Game Name: ',
                           'Password: ',
                           'logintogame',
                           'Login')()

        elif ACTION == 'createnewgame':
            page.LoginPage('Create New Game',
                           'Game Name: ',
                           'Password: ',
                           'createnewgame',
                           'Create Game')()
            
        elif ACTION == 'adminmenu':
            page.MenuPage('Admin Menu',
                          'Add/Remove Beacons',
                          'Manually Assign Player Traits',
                          'Create Custom Traits',
                          'Configure Instructions',
                          'Exit')()

        elif ACTION == 'beaconmenu':
            if 'gamename' and 'pw' in Pars:
                GAMENAME = Pars['gamename'].value
                PW = Pars['gamename'].value
                page.FramePage('Add/Remove Beacons',
                               [objs.DynamicDrop('beaconlist',
                                                 objs.GetBeaconList(GAMENAME, PW),
                                                 'b__blank__beacon',
                                                 ' -- New Beacon --')],
                               'updateaction',
                               'deletebeacon',
                               [['beaconmenu', j.ElementAtt('beaconlist')]],
                               'Update Beacon',
                               'Delete Beacon',
                               'Back')()
            else:
                GoToPage(HOST_ACT + 'startpage')



        #Any Other Page
        else:
            GoToPage(data.HOST_ACT + 'startpage')
