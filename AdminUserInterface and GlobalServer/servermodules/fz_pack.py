def bool(v):
    if v == "False" or v == "0":
        return False
    return True

def ReadFrom(dat):
    R = dat[0]
    dat.remove(dat[0])
    return R

def ReadFromFile(filename):
    return open(filename, 'r').read().split('_f_s_||')
    
def Write(stream, txt):
    stream.write(str(txt) +  '_f_s_||')

