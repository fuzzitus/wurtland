Import data

Global LastP:= 0
Global SList:= [""]

Function ResetP()
	LastP = 0
	SList = [""]
End function

Function Post(stream:TcpStream, url:string)
	stream.WriteLine("GET " + url + " HTTP/1.0")
	stream.WriteLine(String.FromChar(10))
End Function

Function RequestGameList(stream:TcpStream)
	Post(stream, "http://www.fuzzit.us/cgi-bin/GlobalServer.py?action=mobilegetgamelist")
End Function

Function ReadString:string(stream:Stream)
	Local am:= stream.ReadInt(), txt:= ""
	For Local EC:= 0 To am - 1
		txt += String.FromChar(stream.ReadByte())
	Next
	Return txt
End Function

Function ReadProtocol(stream:TcpStream)
	While stream.ReadAvail()
		_readp(stream)
	Wend
End Function

	Function _readp(stream:TcpStream)
		'Clean The Garbage
		Repeat
			Local Dat:= stream.ReadLine()
			If Dat = "__server__protocol__"
				Exit
			Endif
		Forever
		Local Kind:= stream.ReadByte()
		Select Kind
			Case 4'List of Games
				Local am:= stream.ReadByte()
				SList = SList.Resize(am)
				For Local es:= 0 To am - 1
					SList[es] = ReadString(stream)
				Next
		End select
	End function