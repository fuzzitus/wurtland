#Copyright (c) 2013 CageInfamous(tm)

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
