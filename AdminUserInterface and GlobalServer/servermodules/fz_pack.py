import base64

def bool(v):
    if v == "False" or v == "0":
        return False
    return True

def ReadFrom(dat):
    R = dat[0]
    dat.remove(dat[0])
    return R

def ReadFromFile(filename):
    return base64.b64decode(open('/home4/fuzzitus/public_html/' + filename, 'rb').read()).split('__fuzz__split__||')
    
def Write(stream, txt):
    stream.write(base64.b64encode(str(txt) +  '__fuzz__split__||'))
