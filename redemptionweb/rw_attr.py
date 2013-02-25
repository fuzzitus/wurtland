def DivBackgroundImage(image, w, h):
    if isinstance(w, int) or isinstance(w, float):
        W = str(w) + 'px'
    else:
        W = w
    if isinstance(h, int) or isinstance(h, float):
        H = str(h) + 'px'
    else:
        H = h 
    return dict(style='background-image: url(' + image + '); width: ' + W + '; height: ' + H + ';')

def Merge(attrs):
    am = len(attrs)
    aa = attrs[0]
    for EA in range(1, am):
        aa.update(attrs[EA])
    return aa

def toCssStyle(style):
    text = ''
    for EK in style:
        text += EK + ':' + style[EK] + ';'
    return text
