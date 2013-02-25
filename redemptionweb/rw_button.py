import rw_widget as w

class Button(w.Widget):
    def __call__(b):
        txt = '<button name="' + b.Name + '"'
        if b.Attr <> None:
            for EK in b.Attr:
                txt += ' ' + EK + '="' + b.Attr[EK] + '"'
        print txt + '>' + b.Caption + '</button>'
    def __init__(b, name, caption, attr = None):
        b.Name = name
        b.Caption = caption
        b.Attr = attr
