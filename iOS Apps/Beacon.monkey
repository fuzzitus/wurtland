Import data

Class Beacon Extends BApp
	
	Method OnCreate()
		CHGUI_MobileMode = 1
		SetUpdateRate(30)
	End Method
	
	Method OnRender()
		Select STATUS
			Case "connecting"
				RequestGameList()
				STATUS = "normal"
			Case "normal"
				ReadProtocol()
				
				Cls(247, 247, 247)
				
				CHGUI_Draw()
			Case "start"
				CHGUI_Start()
				
				Title = CScale(CreateLabel(50, 10, "Beacon Config"))
				ServerLabel = CScale(CreateLabel(5, 60, "Server Type: Static"))
				Games = CScale(CreateDropdown(10, 110, SCALE_W - 20, 40, "Choose Game"))
				PwLabel = CScale(CreateLabel(5, 160, "Password:"))
				Pw = CScale(CreateTextfield(120, 155, 170, 45, ""))
				BeaconList = CScale(CreateDropdown(10, 210, SCALE_W - 20, 40, "Choose Beacon"))
				On_Off = CScale(CreateButton(10, SCALE_H - 50, SCALE_W - 20, 40, "On/Off"))
				
				isOn = False
				Game = Self
				LastGame = "Choose Game"
				STATUS = "connecting"
		End Select
	End Method
	
	Method OnUpdate()
		Select STATUS
			Case "connecting"
				CHGUI_Update()
			Case "normal"
				CHGUI_Update()
				If LastGame <> Games.Text
					CHGUI_Delete(BeaconList)
					BeaconList = CScale(CreateDropdown(10, 210, SCALE_W - 20, 40, "Choose Beacon"))
					LastGame = Games.Text
					RequestBeaconList(LastGame)
				endif
		End Select
	End Method
	
End Class

Function Main()
	New Beacon
End function