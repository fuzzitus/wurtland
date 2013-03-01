import ciPack as p

# A Null type, it's just good to have one...
def Null():
    print p.Byte(0)

def Error(stream, Type, msg = ''):
    print p.Byte(1)
    print p.Byte(Type)
    print p.String(msg)

def WrongGameName(stream):
    print p.Byte(2)

def WrongPassword(stream):
    print p.Byte(3)
