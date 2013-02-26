import os
import sys

import ciPack as p

class Beacon:
    def __init__(b, Id, name, instructions, activated, phoneid = ''):
        b.Id = Id
        b.Name = name
        b.Instructions = instructions
        b.Activated = activated
        b.PhoneId = phoneid
    def Save(b):
        ff = open(os.getcwd() + '/beacons/' + b.Id + '.fuz', 'wb')
        b.Write(ff)
        ff.close()
    def Send(b, stream):
        p.SendByte(stream, 0)#FORMAT
        p.SendString(stream, b.Id)
        p.SendString(stream, b.Name)
        p.SendInt(stream, len(b.Instructions))
        for EI in b.Instructions:
            p.SendString(stream, EI)
        p.SendBool(stream, b.Activated)
        p.SendString(stream, b.PhoneId)
    def Write(b, stream):
        p.WriteByte(stream, 0)#FORMAT
        p.WriteString(stream, b.Id)
        p.WriteString(stream, b.Name)
        p.WriteInt(stream, len(b.Instructions))
        for EI in b.Instructions:
            p.WriteString(stream, EI)
        p.WriteBool(stream, b.Activated)
        p.WriteString(stream, b.PhoneId)

def RecvBeacon(stream):
    form = p.RecvByte(stream)
    Id = p.RecvString(stream)
    Name = p.RecvString(stream)
    am = p.RecvInt(stream)
    I = []
    for EI in range(0, am):
        I.append(p.RecvString(stream))
    a = p.RecvBool(stream)
    pid = p.RecvString(stream)
    return Beacon(Id, Name, I, a, pid)

def ReadBeacon(stream):
    form = p.ReadByte(stream)
    Id = p.ReadString(stream)
    Name = p.ReadString(stream)
    am = p.ReadInt(stream)
    I = []
    for EI in range(0, am):
        I.append(p.ReadString(stream))
    a = p.ReadBool(stream)
    pid = p.ReadString(stream)
    return Beacon(Id, Name, I, a, pid)

def LoadBeacon(Id):
    ff = open(os.getcwd() + '/beacons/' + Id + '.fuz', 'rb')
    b = ReadBeacon(ff)
    ff.close()
    return b