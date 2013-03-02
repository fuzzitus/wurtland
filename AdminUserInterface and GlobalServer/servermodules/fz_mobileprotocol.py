import os
import ciPack2 as p
import fz_game as g

# A Null type, it's just good to have one...
def Null():
    print '__server__protocol__'
    print 0

def Error(Type, msg = ''):
    print '__server__protocol__'
    print 1
    print p.Byte(Type)
    print msg

def WrongGameName():
    print '__server__protocol__'
    print 2

def WrongPassword():
    print '__server__protocol__'
    print 3

def SendGameList():
    GAMES = os.listdir(g.GAME_LOCATION)
    print '__server__protocol__'
    print 4
    print str(len(GAMES))
    for EG in GAMES:
        print EG.replace('.fuz', '')

def SendBeaconList(game):
    print '__server__protocol__'
    print 5
    GAME = g.LoadGame(game)
    print str(len(GAME.Beacons))
    for EB in GAME.Beacons:
        print EB.Name
