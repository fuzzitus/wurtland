Import data

Class Beacon Extends App

	'WIDGETS
	Field Title:CHGUI'The Title
	Field ServerLabel:CHGUI
	Field Games:CHGUI
	Field PwLabel:CHGUI, Pw:CHGUI
	Field BeaconList:CHGUI
	Field On_Off:CHGUI
	
	'Status
	Field isOn:Bool
	
	'Net
	Field Server:TcpStream
	
	Method OnCreate()
		CHGUI_MobileMode = 1
		SetUpdateRate(30)
	End Method
	
	Method OnRender()
		Select STATUS
			Case "connecting"
				If Server.Connect("www.fuzzit.us", 80)
					RequestGameList(Server)
					While LastP <> 4'Until we get the list
						ReadProtocol(Server)
					Wend
					For Local ES:= Eachin SList
						CreateDropdownItem(ES, Games)
					Next
					STATUS = "normal"
				Endif
			Case "normal"
				Cls(247, 247, 247)
				CHGUI_Draw()
			Case "start"
				Title = CScale(CreateLabel(50, 10, "Beacon Config"))
				ServerLabel = CScale(CreateLabel(5, 60, "Server Type: Static"))
				Games = CScale(CreateDropdown(10, 110, SCALE_W - 20, 40, "Choose Game"))
				PwLabel = CScale(CreateLabel(5, 160, "Password:"))
				Pw = CScale(CreateTextfield(120, 155, 170, 45, ""))
				BeaconList = CScale(CreateDropdown(10, 210, SCALE_W - 20, 40, "Choose Beacon"))
				On_Off = CScale(CreateButton(10, SCALE_H - 50, SCALE_W - 20, 40, "On/Off"))
				
				isOn = False
				Server = New TcpStream
				STATUS = "connecting"
		End Select
	End Method
	
	Method OnUpdate()
		Select STATUS
			Case "normal"
				CHGUI_Update()
		End Select
	End Method
	
End Class

Function Main()
	New Beacon
End function