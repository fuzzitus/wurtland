import rw_widget as w

class Link(w.Widget):
    def __call__(w):
        print '<a href="' + w.Link + '">'
        for EO in w.Objs:
            EO()
        print '</a>'
    def __init__(w, link, objs = []):
        w.Link = link
        w.Objs = objs
