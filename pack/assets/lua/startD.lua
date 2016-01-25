function RunTest()
	MesBox("main.lua loaded!")	-- toast

	-- message box
	if MessageBox("yes no box", "yesno") ==  "yes" then
		MessageBox("yes!", "ok")
	else
		MessageBox("no!", "ok")
	end
end

AddMenu("Debug", "RunTest()")

dofile('lua/start.lua')
