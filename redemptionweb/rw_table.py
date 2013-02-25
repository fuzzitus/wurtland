import rw_widget as w

class Table(w.Widget):
    def __call__(t):
        print '<table border="' + str(t.Border) + '">'
        for eRow in t.Cells:
            print '<tr>'
            for eCell in eRow:
                print '<td>'
                eCell()
                print '</td>'
            print '</tr>'
        print "</table>"
    def __init__(t, Id, cells, border = 1):
        t.Id = Id
        t.Border = border
        t.Cells = cells
