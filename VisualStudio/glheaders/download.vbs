Set objXML = WScript.CreateObject("MSXML2.XMLHTTP.3.0")
Set objStream = Wscript.CreateObject("ADODB.Stream")

objXML.open "GET", "https://www.khronos.org/registry/OpenGL/api/GL/wglext.h", False
objXML.send

objStream.Open
objStream.Type = 1
objStream.Write objXML.responseBody
objStream.SaveToFile "wglext.h", 2
objStream.Close

objXML.open "GET", "https://www.khronos.org/registry/OpenGL/api/GL/glcorearb.h", False
objXML.send

objStream.Open
objStream.Type = 1
objStream.Write objXML.responseBody
objStream.SaveToFile "glcorearb.h", 2
objStream.Close
