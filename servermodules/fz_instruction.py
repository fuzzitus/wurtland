import ciPack2 as p

class Instruction:
    def __init__(i, Id, trait, text):
        i.Id = Id
        i.Trait = trait
        i.Text = text
    def Write(i, stream):
        p.WriteString(stream, i.Id)
        p.WriteString(stream, i.Trait)
        p.WriteString(stream, i.Text)

def ReadInstruction(stream):
    Id = p.ReadString(stream)
    Trait = p.ReadString(stream)
    Text = p.ReadString(stream)
    return Instruction(Id, Trait, Text)
