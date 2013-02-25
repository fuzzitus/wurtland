import rw_widget as w

class Form(w.Widget):
    def __call__(f):
        txt = '<form'
        for EK in f.Attr:
            txt += ' ' + EK + '="' + f.Attr[EK] + '"'
        print txt + '>'
        for EO in f.Objs:
            EO()
        print '</form>'
    def __init__(f, attr = None, objs = []):
        if attr == None:
            f.Attr = dict()
        else:
            f.Attr = attr
        f.Objs = objs

class TextField(w.Widget):
    def __call__(tf):
        print tf.Prompt + '<input type="text" name="' + tf.Name + '">'
    def __init__(tf, name, prompt):
        tf.Name = name
        tf.Prompt = prompt

class PasswordField(w.Widget):
    def __call__(pf):
        print pf.Prompt + '<input type="password" name="' + pf.Name + '">'
    def __init__(pf, name, prompt):
        pf.Name = name
        pf.Prompt = prompt

class SubmitButton(w.Widget):
    def __call__(sb):
        print '<input type="submit" value="' + sb.Caption + '">'
    def __init__(sb, caption):
        sb.Caption = caption

class Radio(w.Widget):
    def __call__(r):
        for EV in r.Values:
            print '<input type="radio" name="' + r.Name + '" value="' + EV[0] + '">' + EV[1]
    def __init__(r, name, values):
        r.Name = name
        r.Values = values#[[value, caption]]

class Checkbox(w.Widget):
    def __call__(cb):
        for EV in r.Values:
            print '<input type="checkbox" name="' + r.Name + '" value="' + EV[0] + '">' + EV[1]
    def __init__(cb, name, values):
        r.Name = name
        r.Values = values#[[value, caption]]
