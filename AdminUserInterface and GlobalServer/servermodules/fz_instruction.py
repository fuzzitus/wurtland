import fz_pack as p

class Instruction:
    def __init__(i, Id, trait, text):
        i.Id = Id
        i.Trait = trait
        i.Text = text
    def Write(i, stream):
        p.Write(stream, i.Id)
        p.Write(stream, i.Trait)
        p.Write(stream, i.Text)

def ReadInstruction(stream):
    Id = p.ReadFrom(stream)
    Trait = p.ReadFrom(stream)
    Text = p.ReadFrom(stream)
    return Instruction(Id, Trait, Text)
