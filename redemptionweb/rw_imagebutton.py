import rw_widget as w

class ImageButton(w.Widget):
    def __call__(b):
        print '<a href="' + b.Link + '"><img src="' + b.Images[0] + '" onmouseover="this.src=' + chr(39) + b.Images[1] + chr(39) + '" onmouseout="this.src=' + chr(39) + b.Images[0] + chr(39) + '" alt="' + b.Id + '"/></a>'
    def __init__(b, Id, images, link):
        b.Id = Id
        b.Images = images
        b.Link = link
