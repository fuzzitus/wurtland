Import challengergui
Import mojo
Import brl
Import protocol

Global STATUS:= "start"

Global SCALE_H:= 480.0, SCALE_W:= 300.0

Function CCenter:CHGUI(c:CHGUI)
	c.X = (Float(DeviceWidth()) / 2.0) - (c.W / 2.0)
	Return c
End Function

Function CScale:CHGUI(c:CHGUI)
	c.X *= (Float(DeviceWidth()) / SCALE_W)
	c.W *= (Float(DeviceWidth()) / SCALE_W)
	c.Y *= (Float(DeviceHeight()) / SCALE_H)
	c.H *= (Float(DeviceHeight()) / SCALE_H)
	Return c
End Function