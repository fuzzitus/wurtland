import os
import sys

import ciPack2 as p

class Beacon:
    def __init__(b, Id, name, instructions, activated, phoneid = ''):
        b.Id = Id
        b.Name = name
        b.Instructions = instructions
        b.Activated = activated
        b.PhoneId = phoneid
    def Write(b, stream):
        p.WriteByte(stream, 0)#FORMAT
        p.Write(stream, b.Id)
        p.Write(stream, b.Name)
        p.WriteInt(stream, len(b.Instructions))
        for EI in b.Instructions:
            p.Write(stream, EI)
        p.WriteBool(stream, b.Activated)
        p.Write(stream, b.PhoneId)

def ReadBeacon(stream):
    form = p2.ReadByte(stream)
    Id = p2.Read(stream)
    Name = p2.Read(stream)
    am = p2.ReadInt(stream)
    I = []
    for EI in range(0, am):
        I.append(p2.Read(stream))
    a = p2.ReadBool(stream)
    pid = p2.Read(stream)
    return Beacon(Id, Name, I, a, pid)
