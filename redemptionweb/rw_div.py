import rw_widget as w

class Div(w.Widget):
    def __call__(w):
        txt = '<div '
        for EA in w.Attr:
            txt += ' ' + EA + '="' + w.Attr[EA] + '"'
        txt += ">"
        print txt
        for EO in w.Objs:
            EO()
        print "</div>"
    def __init__(w, attr, objs):
        w.Attr = attr
        w.Objs = objs
