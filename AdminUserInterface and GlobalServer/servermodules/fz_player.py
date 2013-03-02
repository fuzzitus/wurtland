import os
import sys

import fz_pack

PLAYER_DIR = '/home4/fuzzitus/public_html/players'

def Format(txt):
    return txt.replace('/', '_00_fuz_').replace('?', '_01_fuz_').replace('<', '_02_fuz_').replace('>', '_03_fuz_').replace(chr(92), '_04_fuz_').replace(':', '_05_fuz_').replace('*', '_06_fuz_').replace('"', '_07_fuz_').replace("'", '_08_fuz_').replace(' ', '_09_fuz_')

def FormatBack(txt):
    return txt.replace('_00_fuz_', '/').replace('_01_fuz_', '?').replace('_02_fuz_', '<').replace('_03_fuz_', '>').replace('_04_fuz_', chr(92)).replace('_05_fuz_', ':').replace('_06_fuz_', '*').replace('_07_fuz_', '"').replace('_08_fuz_', "'").replace('_09_fuz_', ' ')


class Player:
    def __init__(p, Id, pw, Traits):
        p.Id = Id
        p.Password = pw
        p.Traits = Traits
    def Save(p):
        ff = open(PLAYER_DIR + '/' + Format(p.Id) + '.fuz', 'w')
        p.Write(ff)
        ff.close()
    def Write(p, stream):
        p2.Write(stream, 0)#FORMAT - This will help for future expansions
        p2.Write(stream, p.Id)
        p2.Write(stream, p.Password)
        p2.Write(stream, len(p.Traits))
        for ET in p.Traits:
            p2.Write(stream, ET[0])
            p2.Write(stream, ET[1])

def ReadPlayer(stream):
    form = int(p2.Read(stream))
    Id = p2.Read(stream)
    Pw = p2.Read(stream)
    am = int(p2.Read(Stream))
    traits = []
    for ET in range(0, am):
        traits.append([p2.Read(stream), ''])
        traits[ET][1] = p2.Read(stream)
    return Player(Id, Pw, traits)

def LoadPlayer(Id):
    ff = open(PLAYER_DIR + '/' + Format(Id) + '.fuz', 'rb')
    P = ReadPlayer(ff)
    ff.close()
    return P

def CheckPlayer(name):
    if os.path.exists(PLAYER_DIR + '/' + Format(name) + '.fuz'):
        return True
    else:
        return False
