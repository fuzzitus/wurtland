from ac_data import *
import fz_game as g

class DynamicDrop(w.Widget):
    def __call__(d):
        d.LOAD_FUNC()
        d.DROP_FUNC()
        print '\n'
        d.DROP()
        print '\n'
        d.FIELD()
    def __init__(d, name, fields, blankfieldid, blankfieldcaption):
        d.LOAD_FUNC = t.T("<script language='text/javascript'>\n" + "document.getElementById('" + name + "').value = '" + blankfieldid + "';\n</script>\n")
        d.DROP_FUNC = t.T("<script language='javascript'>\nfunction Conv_" + name + "()\n" + chr(9) + "{\n" + chr(9) + chr(9) + "if (document.getElementById('" + name + "').value == '" + blankfieldid + "')\n" + chr(9) + chr(9) + chr(9) + "{\n" + chr(9) + chr(9) + chr(9) + chr(9) + "document.getElementById('" + blankfieldid + "').value = '';\n" + chr(9) + chr(9) + chr(9) + "}\n" + chr(9) + chr(9) + "else\n" + chr(9) + chr(9) + chr(9) + "{\n" + chr(9) + chr(9) + chr(9) + chr(9) + "document.getElementById('" + blankfieldid + "').value = document.getElementById('" + name + "').value;\n" + chr(9) + chr(9) + chr(9) + "}\n" + chr(9) + "}\n</script>")
        d.DROP = f.Dropdown(name, [], dict(onchange='javascript:Conf_' + name + '()'))
        for eF in fields:
            if isinstance(eF, str):
                d.FIELD.Values.append([eF, eF])
            else:
                d.FIELD.Values.append([eF[0], eF[0], eF[1]])
        d.FIELD = f.TextField(blankfieldid, blankfieldcaption)


def GetBeaconList(gamename, pw):
    if g.GameExists(gamename):
        if g.CheckPassword(gamename, pw):
            GAME = g.LoadGame(gamename)
            return GAME.Beacons
        else:
            GoToPage(HOST_ACT + 'startpage')
    else:
        GoToPage(HOST_ACT + 'startpage')
