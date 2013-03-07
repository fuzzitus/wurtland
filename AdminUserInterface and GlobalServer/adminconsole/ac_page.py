from ac_data import *

class Page(w.Widget):
    def checkpars(p, pars):
        for eP in pars:
            if eP not in Pars:
                GoToPage(HOST_ACT + 'startpage')
                return False
        return True

class StartPage(Page):
    def __call__(p):
        div.Div(dict(id='main',
                 align='center',
                 style=a.toCssStyle(dict(position='relative',
                                         top='35%'))), [p.Logo, t.br,
                                                        p.Login, t.br,
                                                        p.NewGame])()
    def __init__(p, logo, logintext, newgametext):
        p.Logo = i.Image('logo', GFX_DIR + '/' + logo)
        p.Login = l.Link(HOST_ACT + 'logintogame', [b.Button('logintogame', logintext)])
        p.NewGame = l.Link(HOST_ACT + 'createnewgame', [b.Button('createnewgame', newgametext)])


class LoginPage(Page):
    def __call__(p):
        if 'message' in Pars:
            M = Pars['message'].value
        else:
            M = ''
        div.Div(dict(id='main',
                 align='center',
                 style=a.toCssStyle(dict(position='relative',
                                         top='40%'))),
                [p.TITLE, t.br,
                 f.Form(dict(action = SERVER),
                        [p.GAME_NAME, t.br,
                         p.PW, t.br, p.ACT,
                         p.BUTTON]), t.p,
                 t.Text(M, dict(color='red'))])()
                
    def __init__(p, title, gamename_f, pw_f, action, submit):
        p.TITLE = t.Text(title, dict(size='36'))
        p.GAME_NAME = f.TextField('gamename', gamename_f)
        p.PW = f.TextField('pw', pw_f)
        p.ACT = f.Hidden('action', action)
        p.BUTTON = f.SubmitButton(submit)

class MenuPage(Page):
    def __call__(p):
        if p.checkpars(['gamename', 'pw']):
            div.Div(dict(id='main',
                     align='center',
                     style=a.toCssStyle(dict(position='relative',
                                             top='40%'))),
                    [p.TITLE, t.p,
                     p.BEACON, t.br,
                     p.PLAYER, t.br,
                     p.INS, t.p,
                     p.EXIT])()
    def __init__(p, title, beacont, playert, traitst, inst, exitt):
        if p.checkpars(['gamename', 'pw']):
            GAMENAME = Pars['gamename'].value
            PW = Pars['pw'].value
            p.TITLE = t.Text(title, dict(size='36'))
            p.BEACON = l.Link(HOST_ACT + 'beaconmenu&gamename=' + GAMENAME + '&pw=' + PW, [b.Button('arbeacons', beacont)])
            p.PLAYER = l.Link(HOST_ACT + 'playertraits&gamename=' + GAMENAME + '&pw=' + PW, [b.Button('playertraits', playert)])
            p.TRAITS = l.Link(HOST_ACT + 'traits&gamename=' + GAMENAME + '&pw=' + PW, [b.Button('traits', traitst)])
            p.INS = l.Link(HOST_ACT + 'ins&gamename=' + GAMENAME + '&pw=' + PW, [b.Button('ins', inst)])
            p.EXIT = l.Link(HOST_ACT + 'startpage', [b.Button('exit', exitt)])

class FramePage(Page):
    def __call__(p):
        if p.checkargs(['gamename', 'pw']):
            O = [p.ACTION, t.br]
            for eO in p.OBJS:
                O.append(eO)
            O.append(p.UPDATE)
            div.Div(dict(id='main',
                     align='center',
                     style=a.toCssStyle(dict(position='relative',
                                             top='40%'))),
                    [p.TITLE, t.p,
                     f.Form(dict(align='left',
                                 action=SERVER), O), t.br,
                     p.DEL_ACTION, t.T('\n'),
                     p.DELETE, t.br,
                     p.EXIT])()
    def __init__(p, title, objs, action, delactionname, delactionargs, update, delete, exitb):
        if p.checkargs(['gamename', 'pw']):
            p.GAMENAME = Pars['gamename'].value
            p.PW = Pars['pw'].value
            p.TITLE = t.Text(title, dict(size='36'))
            p.OBJS = objs
            p.ACTION = f.Hidden('action', action)
            T = SERVER_ACT + delaction + '&gamename=' + p.GAMENAME + '&pw=' + p.PW
            for eA in delactionargs:
                T += '&' + eA[0] + '=' + eA[1]
            p.DEL_ACTION = j.Func('Del', [], T +';')
            p.UPDATE = f.SubmitButton(update)
            p.DELETE = b.Button('delete', delete, dict(action='javascript:Del()'))
            p.EXIT = l.Link(HOST_ACT + 'adminmenu&gamename=' + p.GAMENAME + '&pw=' + p.PW, [b.Button('exit', exitb)])
