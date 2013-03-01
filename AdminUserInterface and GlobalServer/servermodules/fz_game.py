import os
import sys

import fz_pack as p

import fz_beacon as b
import fz_instruction as i

GAME_LOCATION = '/home4/fuzzitus/public_html/games'

def Format(txt):
    return txt.replace('/', '_00_fuz_').replace('?', '_01_fuz_').replace('<', '_02_fuz_').replace('>', '_03_fuz_').replace(chr(92), '_04_fuz_').replace(':', '_05_fuz_').replace('*', '_06_fuz_').replace('"', '_07_fuz_').replace("'", '_08_fuz_').replace(' ', '_09_fuz_')

def FormatBack(txt):
    return txt.replace('_00_fuz_', '/').replace('_01_fuz_', '?').replace('_02_fuz_', '<').replace('_03_fuz_', '>').replace('_04_fuz_', chr(92)).replace('_05_fuz_', ':').replace('_06_fuz_', '*').replace('_07_fuz_', '"').replace('_08_fuz_', "'").replace('_09_fuz_', ' ')

class Game:
    def __init__(g, Id, password, gamestarted, beacons, players, traits, instructions):
        g.Id = Id
        g.Password = password
        g.GameStarted = gamestarted
        g.Beacons = beacons
        g.Players = players#Player Ids
        g.Traits = traits
        g.Instructions = instructions
    def GetBeacon(name):
        for EB in Beacons:
            if EB.Id == name:
                return EB
        return None
    def Save(g):
        ff = open(GAME_LOCATION + '/' + Format(g.Id) + '.fuz', 'wb')
        g.Write(ff)
        ff.close()
    def Write(g, stream):
        p.Write(stream, 0)#FORMAT
        p.Write(stream, g.Id)
        p.Write(stream, g.Password)
        p.Write(stream, g.GameStarted)
        p.Write(stream, len(g.Beacons))
        for EB in g.Beacons:
            EB.Write(stream)
        p.Write(stream, len(g.Players))
        for EP in g.Players:
            p.Write(EP)
        p.Write(stream, len(g.Traits))
        for ET in g.Traits:
            p.Write(ET)
        p.Write(stream, len(g.Instructions))
        for EI in g.Instructions:
            p.Write(EI)

def ReadGame(stream):
    form = p.ReadFrom(stream)
    Id = p.ReadFrom(stream)
    pw = p.ReadFrom(stream)
    gs = p.ReadFrom(stream)
    am = p.ReadFrom(stream)
    B = []
    for EA in range(am):
        B.append(b.ReadBeacon(stream))
    am = p.ReadFrom(stream)
    P = []
    for EA in range(am):
        P.append(p.ReadFrom(stream))
    am = p.ReadFrom(stream)
    T = []
    for EA in range(am):
        T.append(p.ReadFrom(stream))
    I = []
    for EI in range(am):
        I.append(i.ReadInstruction(stream))
    return Game(Id, pw, gs, B, P, T, I)

def LoadGame(Id):
    ff = ReadFromFile('/games/' + Format(Id) + '.fuz', 'rb')
    g = ReadGame(ff)
    ff.close()
    return g

def CheckGameExists(name):
    if os.path.exists(GAME_LOCATION + '/' + Format(name) + '.fuz'):
        return True
    else:
        return False

def CheckPassword(game, pw):
    if CheckGameExists(game):
        F = open(GAME_LOCATION + '/' + Format(game) + '.fuz', 'rb')
        p.ReadInt()
        N = p.Read()
        PW = p.Read()
        F.close()
        if PW == pw:
            return True
        else:
            return False
    else:
        return False
