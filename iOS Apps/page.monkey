Import data

Global CUR_PAGE:Page

Class Page
	Method Render()
	End Method
	Method Update()
	End Method
	Method Clear()
	End method
End Class

Class SplashPage Extends Page
	Field Title:CHGUI
	Field Login:CHGUI
	Field LoginFB:CHGUI
	Field NewAccount:CHGUI
	Method New()
		Title = CScale(CreateLabel(80, 10, "Beacon Demo"))
		Login = CScale(CreateButton(20, 100, SCALE_W - 40, 50, "Login"))
		LoginFB = CScale(CreateButton(20, 160, SCALE_W - 40, 50, "Facebook Login"))
		NewAccount = CScale(CreateButton(20, 220, SCALE_W - 40, 50, "Create A New Account"))
	End Method
	Method Render()
		CHGUI_Draw()
	End Method
	Method Update()
		CHGUI_Update()
		If NewAccount.Clicked
			Clear()
			CUR_PAGE = New CreateAccountPage()
		Endif
		If Login.Clicked
			Clear()
			CUR_PAGE = New LoginPage()
		Endif
	End Method
	Method Clear()
		CHGUI_Delete(Title)
		CHGUI_Delete(Login)
		CHGUI_Delete(LoginFB)
		CHGUI_Delete(NewAccount)
	End method
End Class

Class CreateAccountPage Extends Page
	Field Title:CHGUI
	Field FirstName:CHGUI, T_FN:CHGUI
	Field LastName:CHGUI, T_LN:CHGUI
	Field Username:CHGUI, T_UN:CHGUI
	Field Pw:CHGUI,  T_PW:CHGUI
	Field Interests:CHGUI, T_I:CHGUI
	Field Start:CHGUI
	Method New()
		Title = CScale(CreateLabel(80, 10, "Create Account"))
		FirstName = CScale(CreateLabel(5, 60, "First Name:"))
		LastName = CScale(CreateLabel(5, 100, "Last Name:"))
		Username = CScale(CreateLabel(5, 140, "Username:"))
		Pw = CScale(CreateLabel(5, 180, "Password:"))
		Interests = CScale(CreateLabel(5, 220, "Interests:"))
		
		T_FN = CScale(CreateTextfield(10 + CHGUI_Font.GetTxtWidth(FirstName.Text), 55, 140, 35, ""))
		T_LN = CScale(CreateTextfield(10 + CHGUI_Font.GetTxtWidth(FirstName.Text), 95, 140, 35, ""))
		T_UN = CScale(CreateTextfield(10 + CHGUI_Font.GetTxtWidth(FirstName.Text), 135, 140, 35, ""))
		T_PW = CScale(CreateTextfield(10 + CHGUI_Font.GetTxtWidth(FirstName.Text), 175, 140, 35, ""))
		T_I = CScale(CreateTextfield(10 + CHGUI_Font.GetTxtWidth(FirstName.Text), 215, 140, 35, ""))
		
		Start = CScale(CreateButton(20, SCALE_H - 100, SCALE_W - 40, 50, "Start"))
	End Method
	Method Render()
		CHGUI_Draw()
	End Method
	Method Update()
		CHGUI_Update()
	End method
End Class

Class LoginPage Extends Page
	Field Title:CHGUI
	Field Un:CHGUI, T_Un:CHGUI
	Field Pw:CHGUI, T_Pw:CHGUI
	Field Login:CHGUI
	Method New()
		Title = CScale(CreateLabel(100, 10, "Login"))
		Un = CScale(CreateLabel(5, 60, "Username:"))
		Pw = CScale(CreateLabel(5, 100, "Password:"))
		T_Un = CScale(CreateTextfield(10 + CHGUI_Font.GetTxtWidth(Un.Text), 55, 140, 35, ""))
		T_Pw = CScale(CreateTextfield(10 + CHGUI_Font.GetTxtWidth(Un.Text), 95, 140, 35, ""))
		Login = CScale(CreateButton(20, SCALE_H - 100, SCALE_W - 40, 50, "Login"))
	End Method
	Method Render()
		CHGUI_Draw()
	End Method
	Method Update()
		CHGUI_Update()
	End method
End Class

Class ActionPage Extends Page
	Field Title:CHGUI
	Field DoThis:CHGUI
	Field Instructions:CHGUI
	Field Settings:CHGUI
	Field EndG:CHGUI
	Method New()
		Title = CScale(CreateLabel(100, 10, "Action Mode"))
		DoThis = CScale(CreateLabel(5, 60, "Do This:"))
		Instructions = CScale(CreateLabel(5, 100, "Instructions:"))
	End Method
End Class