import rw_widget as w

class T(w.Widget):
    def __call__(t):
        print t.Text
    def __init__(t, text):
        t.Text = text

class Text(w.Widget):
    def __call__(t):
        if t.Attr == None:
            print t.Text
        else:
            txt = '<font '
            for EA in t.Attr:
                txt += ' ' + EA + '="' + t.Attr[EA] + '"'
            txt += ">"
            print txt + t.Text + '</font>'
    def __init__(t, text, attr = None):
        t.Attr = attr
        t.Text = text

class Space(w.Widget):
    def __call__(t):
        print '<br>'
    def __init__(t):
        pass

class DoubleSpace(w.Widget):
    def __call__(t):
        print '<p>'
    def __init__(t):
        pass

br = Space()
p = DoubleSpace()
