import os
import sys

import ciPack as pk
import ciPack2 as p2

PLAYER_DIR = '/home4/fuzzitus/public_html/players'

class Player:
    def __init__(p, Id, pw, Traits):
        p.Id = Id
        p.Password = pw
        p.Traits = Traits
    def Save(p):
        ff = open(PLAYER_DIR + '/' + p.Id + '.fuz', 'wb')
        p.Write(ff)
        ff.close()
    def Write(p, stream):
        p2.WriteByte(stream, 0)#FORMAT - This will help for future expansions
        p2.Write(stream, p.Id)
        p2.Write(stream, p.Password)
        p2.WriteInt(stream, len(p.Traits))
        for ET in p.Traits:
            p2.Write(stream, ET[0])
            p2.Write(stream, ET[1])

def ReadPlayer(stream):
    form = p2.ReadByte(stream)
    Id = p2.Read(stream)
    Pw = p2.Read(stream)
    am = p2.ReadInt(Stream)
    traits = []
    for ET in range(0, am):
        traits.append([p2.Read(stream), ''])
        traits[ET][1] = p2.Read(stream)
    return Player(Id, Pw, traits)

def LoadPlayer(Id):
    ff = open(PLAYER_DIR + '/' + Id + '.fuz', 'rb')
    P = ReadPlayer(ff)
    ff.close()
    return P

def CheckPlayer(name):
    if os.path.exists(PLAYER_DIR + '/' + name + '.fuz'):
        return True
    else:
        return False
