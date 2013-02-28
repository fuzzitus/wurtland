import struct as s

#Copyright (c) 2013 CageInfamous(tm)

#Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files
#(the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge,
#publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do
#so, subject to the following conditions:
#The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
#MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
#FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
#WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

def StringPack(txt):
    return str(txt) + "s"

def Byte(num):
    return s.pack('B', num)

def Float(num):
    return s.pack('f', num)

def FromByte(num):
    return s.unpack('B', num)[0]

def FromFloat(num):
    return s.unpack('f', num)[0]

def FromShort(num):
    return s.unpack('H', num)[0]

def FromString(txt):
    am = FromInt(txt[0:4])
    return s.unpack('{}s'.format(len(txt) - 4), txt[4:len(txt)])[0]

def FromInt(num):
    return s.unpack('I', num)[0]

def Int(num):
    return s.pack('I', num)

def ReadByte(stream):
    return FromByte(stream.read(1))

def ReadFloat(stream):
    return FromFloat(stream.read(4))

def ReadInt(stream):
    return FromInt(stream.read(4))

def ReadShort(stream):
    return FromShort(stream.read(2))

def ReadString(stream):
    am = ReadInt(stream)
    return stream.read(am)

def RecvByte(stream):
    return ord(stream.recv(1))

def RecvFloat(stream):
    return FromFloat(stream.recv(4))

def RecvInt(stream):
    return FromInt(stream.recv(4))

def RecvShort(stream):
    return FromShort(stream.recv(2))

def RecvString(stream):
    am = RecvInt(stream)
    return stream.recv(am)

def SendByte(stream, num):
    stream.send(Byte(num))

def SendFloat(stream, num):
    stream.send(Float(num))

def SendInt(stream, num):
    stream.send(Int(num))

def SendShort(stream, num):
    stream.send(Short(num))

def SendString(stream, txt):
    stream.send(String(txt))

def Short(num):
    return s.pack('H', num)

def String(txt):
    return Int(len(txt)) + s.pack('{}s'.format(len(txt)), txt)

def WriteByte(stream, num):
    print >> stream, Byte(num)

def WriteFloat(stream, num):
    print >> stream, Float(num)

def WriteInt(stream, num):
    print >> stream, Int(num)

def WriteShort(stream, num):
    print >> stream, Short(num)

def WriteString(stream, num):
    print >> stream, String(num)
