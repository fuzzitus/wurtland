def ReadFrom(dat):
    R = dat[0]
    dat.remove(dat[0])
    return R

def ReadFromFile(filename):
    return open('/home4/fuzzitus/public_html/' + filename, 'rb').read().split(chr(12) + chr(13) + chr(14))
    
def Write(stream, txt):
    if isinstance(txt, bool):
        if txt:
            stream.write('0' + chr(12) + chr(13) + chr(14))
        else:
            stream.write('1' + chr(12) + chr(13) + chr(14))
    else:
        stream.write(str(txt) + chr(12) + chr(13) + chr(14))
