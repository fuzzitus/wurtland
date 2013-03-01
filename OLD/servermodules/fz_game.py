import os
import sys

import ciPack as p
import ciPack2 as p2

def Format(txt):
    return txt.replace('/', '_00_fuz').replace('?', '_01_fuz').replace('<', '_02_fuz').replace('>', '_03_fuz').replace(chr(92), '_04_fuz').replace(':', '_05_fuz').replace('*', '_06_fuz').replace('"', '_07_fuz').replace("'", '_08_fuz').replace(' ', '_09_fuz')

def FormatBack(txt):
    return txt.replace('_00_fuz', '/').replace('_01_fuz', '?').replace('_02_fuz', '<').replace('_03_fuz', '>').replace('_04_fuz', chr(92)).replace('_05_fuz', ':').replace('_06_fuz', '*').replace('_07_fuz', '"').replace('_08_fuz', "'").replace('_09_fuz', ' ')

class Game:
    def __init__(g, Id, password, gamestarted, beacons, players, traits):
        g.Id = Id
        g.Password = password
        g.GameStarted = gamestarted
        g.Beacons = beacons#Beacon Ids
        g.Players = players#Player Ids
        g.Traits = traits
    def Save(g):
        ff = open('/home4/fuzzitus/public_html/games/' + Format(g.Id) + '.fuz', 'wb')
        g.Write(ff)
        ff.close()
    def Send(g, stream):
        p.SendByte(stream, 0)#FORMAT
        p.SendString(stream, g.Id)
        p.SendString(stream, g.Password)
        p.SendBool(stream, g.GameStarted)
        p.SendInt(stream, len(g.Beacons))
        for EB in g.Beacons:
            p.SendString(EB)
        p.SendInt(stream, len(g.Players))
        for EP in g.Players:
            p.SendString(EP)
        p.SendInt(stream, len(g.Traits))
        for ET in g.Traits:
            p.SendString(ET)
    def Write(g, stream):
        p2.WriteByte(stream, 0)#FORMAT
        p2.Write(stream, g.Id)
        p2.Write(stream, g.Password)
        p2.WriteBool(stream, g.GameStarted)
        p2.WriteInt(stream, len(g.Beacons))
        for EB in g.Beacons:
            p2.Write(EB)
        p2.WriteInt(stream, len(g.Players))
        for EP in g.Players:
            p2.Write(EP)
        p2.WriteInt(stream, len(g.Traits))
        for ET in g.Traits:
            p2.Write(ET)

def RecvGame(stream):
    form = p.RecvByte(stream)
    Id = p.RecvString(stream)
    pw = p.RecvString(stream)
    gs = p.RecvBool(stream)
    am = p.RecvInt(stream)
    B = []
    for EA in range(0, am):
        B.append(p.RecvString(stream))
    am = p.RecvInt(stream)
    P = []
    for EA in range(0, am):
        P.append(p.RecvString(stream))
    am = p.RecvInt(stream)
    T = []
    for EA in range(0, am):
        T.append(p.RecvString(stream))
    return Game(Id, pw, gs, B, P, T)

def ReadGame(stream):
    form = p2.ReadByte(stream)
    Id = p2.Read(stream)
    pw = p2.Read(stream)
    gs = p2.ReadBool(stream)
    am = p2.ReadInt(stream)
    B = []
    for EA in range(0, am):
        B.append(p.Read(stream))
    am = p2.ReadInt(stream)
    P = []
    for EA in range(0, am):
        P.append(p.Read(stream))
    am = p2.ReadInt(stream)
    T = []
    for EA in range(0, am):
        T.append(p2.Read(stream))
    return Game(Id, pw, gs, B, P, T)

def LoadGame(Id):
    ff = open('/home4/fuzzitus/public_html/games/' + FormatBack(Id) + '.fuz', 'rb')
    g = ReadGame(ff)
    ff.close()
    return g
