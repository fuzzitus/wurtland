import os
import sys

import ciPack as pk
import ciPack2 as p2

class Player:
    def __init__(p, Id, pw, Traits):
        p.Id = Id
        p.Password = pw
        p.Traits = Traits
    def Save(p):
        ff = open(os.getcwd() + '/playerprofiles/' + p.Id + '.fuz', 'wb')
        p.Write(ff)
        ff.close()
    def Send(p, stream):
        pk.SendByte(stream, 0)#FORMAT - This will help for furtue expansions
        pk.SendString(stream, p.Id)
        pk.SendString(stream, p.Password)
        pk.SendShort(stream, len(p.Traits))
        for ET in p.Traits:
            pk.SendString(stream, ET[0])
            pk.SendString(stream, ET[1])
    def Write(p, stream):
        p2.WriteByte(stream, 0)#FORMAT - This will help for furtue expansions
        p2.Write(stream, p.Id)
        p2.Write(stream, p.Password)
        p2.WriteShort(stream, len(p.Traits))
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

def RecvPlayer(stream):
    form = pk.RecvByte(stream)
    Id = pk.RecvString(stream)
    pw = pk.RecvString(stream)
    am = pk.RecvInt(Stream)
    traits = []
    for ET in range(0, am):
        traits.append([pk.RecvString(stream), ''])
        traits[ET][1] = pk.RecvString(stream)
    return Player(Id, pw, traits)

def LoadPlayer(Id):
    ff = open(os.getcwd() + '/playerprofiles/' + Id + '.fuz', 'rb')
    P = ReadPlayer(ff)
    ff.close()
    return P
