import fz_pack as p

class Instruction:
    def __init__(i, Id, traits, text):
        i.Id = Id
        i.Traits = traits
        i.Text = text
    def Write(i, stream):
        p.Write(stream, i.Id)
        p.Write(stream, len(i.Traits))
	for ET in i.Traits:
		p.Write(stream, ET)
        p.Write(stream, i.Text)

def ReadInstruction(stream):
	Id = p.ReadFrom(stream)
	Traits = []
	am = int(p.ReadFrom(stream))
	for ET in range(am):
		Traits.append(p.ReadFrom(stream))
	Text = p.ReadFrom(stream)
	return Instruction(Id, Traits, Text)
