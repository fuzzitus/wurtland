#Copyright (c) 2013 Nathaniel "CageInfamous" Wilson, cageinfamous@redemptionsoft.com

#Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files
#(the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge,
#publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do
#so, subject to the following conditions:
#The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
#MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
#FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
#WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

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
        print tf.Prompt + '<input type="text" id="' + tf.Name + '" name="' + tf.Name + '">'
    def __init__(tf, name, prompt):
        tf.Name = name
        tf.Prompt = prompt

class PasswordField(w.Widget):
    def __call__(pf):
        print pf.Prompt + '<input type="password" id="' + pf.Name + '" name="' + pf.Name + '">'
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
            print '<input type="radio" id="' + r.Name + '" name="' + tf.Name + '" value="' + EV[0] + '">' + EV[1]
    def __init__(r, name, values):
        r.Name = name
        r.Values = values#[[value, caption]]

class Checkbox(w.Widget):
    def __call__(cb):
        for EV in cb.Values:
            TXT = '<input type="checkbox" id="' + EV[0] + '" name="' + EV[0] + '" value="' + EV[1].replace('<br>', '').replace('<p>', '') + '"'
            if len(EV) > 2:
                TXT += ' checked'
            print TXT + '>' + EV[1]
    def __init__(cb, name, values):
        cb.Name = name
        cb.Values = values#[[value, caption, checked = False]]

class Dropdown(w.Widget):
    def __call__(d):
        TXT = '<select id="' + d.Name + '" name="' + d.Name + '"'
        if d.Attr <> None:
            for EA in d.Attr:
                TXT += ' ' + EA + '="' + d.Attr[EA] + '"'
        print TXT + '>\n'
        for EV in d.Values:
            if len(EV) == 2:
                print '<option value="' + EV[0] + '">' + EV[1] + '</option>\n'
            else:
                T = '<option value="' + EV[0] + '"'
                for EA in EV[2]:
                    T += ' ' + EA + '="' + EV[2][EA] + '"'
                T += ">" + EV[1] + "</option>\n'"
                print T
        print '</select>'
    def __init__(d, name, values = [], attrs = None):
        d.Name = name
        d.Values = values#[[value, caption, attr]]
        d.Attr = attrs

class Hidden(w.Widget):
    def __call__(h):
        print "<input type='hidden' id='" + h.Name + "' name='" + h.Name + "' value='" + h.Value + "'>"
    def __init__(h, name, value):
        h.Name = name
        h.Value = value
