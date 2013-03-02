Import data

Function Post(url:string)
	Server.WriteLine("GET " + url + " HTTP/1.0")
	Server.WriteLine(String.FromChar(10))
End Function

Function RequestGameList()
	Server = New TcpStream
	Repeat
	Until Server.Connect("www.fuzzit.us", 80)
	Post("http://www.fuzzit.us/cgi-bin/GlobalServer.py?action=mobilegetgamelist")
End Function

Function RequestBeaconList(game:String)
	Server = New TcpStream
	Repeat
	Until Server.Connect("www.fuzzit.us", 80)
	Post("http://www.fuzzit.us/cgi-bin/GlobalServer.py?action=mobilegetbeaconlist&game=" + game)
End Function

Function ReadProtocol()
	If Server <> Null
		If Server.ReadAvail()
			While Server.ReadAvail()
				_readp()
			Wend
			Server.Close()
			Server = Null
		endif
	Endif
End Function

	Function _readp()
		'Clean The Garbage
		Repeat
			Local Dat:= Server.ReadLine()
			If Dat = "__server__protocol__"
				Exit
			Endif
		Forever
		If Server.ReadAvail()
			Local Kind:= Int(Server.ReadLine())
			Select Kind
				Case 4'List of Games
					Local am:= Int(Server.ReadLine())
					For Local es:= 0 To am - 1
						CreateDropdownItem(Server.ReadLine(), Game.Games)
					Next
				Case 5'List of Beacons
					Local am:= Int(Server.ReadLine())
					For Local es:= 0 To am - 1
						CreateDropdownItem(Server.ReadLine(), Game.BeaconList)
					Next
			End Select
		endif
	End function