#Copyright (c) 2013 Nathaniel "CageInfamous" Wilson, cageinfamous@gmail.com

#Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files
#(the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge,
#publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do
#so, subject to the following conditions:
#The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
#MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
#FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
#WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#BYTES
def ReadByte(stream):
    return ord(stream.read(1))

def RecvByte(stream):
    return ord(stream.recv(1))

def SendByte(stream, num):
    stream.send(chr(num))
    
def WriteByte(stream, num):
    stream.write(chr(num))

#BOOLS
def ReadBool(stream):
    if ReadByte(stream) == 1:
        return True
    return False

def RecvBool(stream):
    if RecvByte(stream) == 1:
        return True
    return False

def SendBool(stream, val):
    if val:
        SendByte(stream, 1)
    else:
        SendByte(stream, 0)

def WriteBool(stream, val):
    if val:
        WriteByte(stream, 1)
    else:
        WriteByte(stream, 0)

#INTS
def ReadInt(stream, num):
    am = ReadByte(stream)
    return int(stream.read(am), 0)

def RecvInt(stream, num):
    am = RecvByte(stream)
    return int(stream.recv(am), 0)
    
def SendInt(stream, num):
    T = hex(num)
    SendByte(stream, len(T))
    stream.send(T)

def WriteInt(stream, num):
    T = hex(num)
    WriteByte(stream, len(T))
    stream.write(T)

#EVERYTHING ELSE
def Read(stream):
    am = ReadInt(stream)
    return stream.read(am)

def Recv(stream):
    am = RecvInt(stream)
    return stream.recv(am)
    
def Send(stream, txt):
    SendInt(stream, len(str(txt)))
    stream.send(txt)

def Write(stream, txt):
    WriteInt(stream, len(str(txt)))
    stream.write(txt)
