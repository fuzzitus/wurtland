import os
import ciPack as p
import fz_game as g

# A Null type, it's just good to have one...
def Null():
    print '__server__protocol__'
    print p.Byte(0)

def Error(Type, msg = ''):
    print '__server__protocol__'
    print p.Byte(1)
    print p.Byte(Type)
    print p.String(msg)

def WrongGameName():
    print '__server__protocol__'
    print p.Byte(2)

def WrongPassword():
    print '__server__protocol__'
    print p.Byte(3)

def SendGameList():
    print '__server__protocol__'
    print p.Byte(4)
    GAMES = os.listdir(g.GAME_LOCATION)
    print p.Int(len(GAMES))
    for EG in GAMES:
        print p.String(EG.replace('.fuz', ''))
