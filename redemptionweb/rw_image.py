import rw_widget as w

class Image(w.Widget):
    def __call__(i):
        txt = '<img src="' + i.Src + '" alt="' + i.Alt + '"'
        if i.W > 0:
            txt += ' width="' + i.W + '"'
        if i.H > 0:
            txt += ' height="' + i.H + '"'
        txt += ">"
        print txt
    def __init__(i, Id, src, w = 0, h = 0, alt = ''):
        i.Id = Id
        i.Src = src
        i.W = w
        i.H = h
        i.Alt = alt
