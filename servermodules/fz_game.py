import os
import sys

import ciPack as p

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
        ff = open(os.getcwd() + '/games/' + Format(g.Id) + '.fuz', 'wb')
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
        p.WriteByte(stream, 0)#FORMAT
        p.WriteString(stream, g.Id)
        p.WriteString(stream, g.Password)
        p.WriteBool(stream, g.GameStarted)
        p.WriteInt(stream, len(g.Beacons))
        for EB in g.Beacons:
            p.WriteString(EB)
        p.WriteInt(stream, len(g.Players))
        for EP in g.Players:
            p.WriteString(EP)
        p.WriteInt(stream, len(g.Traits))
        for ET in g.Traits:
            p.WriteString(ET)

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
    form = p.ReadByte(stream)
    Id = p.ReadString(stream)
    pw = p.ReadString(stream)
    gs = p.ReadBool(stream)
    am = p.ReadInt(stream)
    B = []
    for EA in range(0, am):
        B.append(p.ReadString(stream))
    am = p.ReadInt(stream)
    P = []
    for EA in range(0, am):
        P.append(p.ReadString(stream))
    am = p.ReadInt(stream)
    T = []
    for EA in range(0, am):
        T.append(p.ReadString(stream))
    return Game(Id, pw, gs, B, P, T)

def LoadGame(Id):
    ff = open(os.getcwd() + '/games/' + FormatBack(Id) + '.fuz', 'rb')
    g = ReadGame(ff)
    ff.close()
    return g
