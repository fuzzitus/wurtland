Import data

Class UserApp Extends App
	
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
					STATUS = "normal"
				Endif
			Case "normal"
				Cls(247, 247, 247)
				CUR_PAGE.Render()
				CHGUI_Draw()
			Case "start"
				Server = New TcpStream
				CUR_PAGE = New SplashPage()
				STATUS = "connecting"
		End select
	End Method
	
	Method OnUpdate()
		Select STATUS
			Case "connecting"
			Case "normal"
				CUR_PAGE.Update()
				CHGUI_Update()
		End Select
	End Method
	
End Class

Function Main()
	New UserApp
End function