import os
import sys

import fz_pack as p

class Beacon:
    def __init__(b, name, x, y, instructions, activated, phoneid = ''):
        b.Name = name
        b.X = x
        b.Y = y
        b.Instructions = instructions
        b.Activated = activated
        b.PhoneId = phoneid
    def Write(b, stream):
        p.Write(stream, 0)#FORMAT
        p.Write(stream, b.Name)
        p.Write(stream, b.X)
        p.Write(stream, b.Y)
        p.Write(stream, len(b.Instructions))
        for EI in b.Instructions:
            p.Write(stream, EI)
        p.Write(stream, b.Activated)
        p.Write(stream, b.PhoneId)

def ReadBeacon(stream):
    form = p.ReadFrom(stream)
    Name = p.ReadFrom(stream)
    X = float(p.ReadFrom(stream))
    Y = float(p.ReadFrom(stream))
    am = int(p.ReadFrom(stream))
    I = []
    for EI in range(am):
        I.append(p.ReadFrom(stream))
    a = p.bool(p.ReadFrom(stream))
    pid = p.ReadFrom(stream)
    return Beacon(Name, X, Y, I, a, pid)
