import os
import sys

import ciPack as pk

class Player:
    def __init__(p, Id, Traits):
        p.Id = Id
        p.Traits = Traits
    def Save(p):
        ff = open(os.getcwd() + '/playerprofiles/' + p.Id + '.fuz', 'wb')
        p.Write(ff)
        ff.close()
    def Send(p, stream):
        pk.SendByte(stream, 0)#FORMAT - This will help for furtue expansions
        pk.SendString(stream, p.Id)
        pk.SendShort(stream, len(p.Traits))
        for ET in p.Traits:
            pk.SendString(stream, ET[0])
            pk.SendString(stream, ET[1])
    def Write(p, stream):
        pk.WriteByte(stream, 0)#FORMAT - This will help for furtue expansions
        pk.WriteString(stream, p.Id)
        pk.WriteShort(stream, len(p.Traits))
        for ET in p.Traits:
            pk.WriteString(stream, ET[0])
            pk.WriteString(stream, ET[1])

def ReadPlayer(stream):
    form = pk.ReadByte(stream)
    Id = pk.ReadString(stream)
    am = pk.ReadInt(Stream)
    traits = []
    for ET in range(0, am):
        traits.append([pk.ReadString(stream), ''])
        traits[ET][1] = pk.ReadString(stream)
    return Player(Id, traits)

def RecvPlayer(stream):
    form = pk.RecvByte(stream)
    Id = pk.RecvString(stream)
    am = pk.RecvInt(Stream)
    traits = []
    for ET in range(0, am):
        traits.append([pk.RecvString(stream), ''])
        traits[ET][1] = pk.RecvString(stream)
    return Player(Id, traits)

def LoadPlayer(Id):
    ff = open(os.getcwd() + '/playerprofiles/' + Id + '.fuz', 'rb')
    P = ReadPlayer(ff)
    ff.close()
    return P
