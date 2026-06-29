workspace( "TestParser" )
	configurations( { "Debug" } )
	startproject( "TestApp" )
	
project( "Parser" )
	kind( "StaticLib" )
	location( "Parser" )
	language( "C++" )
	files(
	{
		"Parser/src/**.h",
		"Parser/src/**.cpp"
	} )
	filter( "configurations:Debug" )
		targetdir( "bin/Debug" )
		defines( "DEBUG" )
		symbols( "On" )
		fatalwarnings { "All" }
	
project( "TestApp" )
	kind( "ConsoleApp" )
	location( "TestApp" )
	language( "C++" )
	links( "Parser" )
	includedirs( "Parser/src" )
	debugdir( "TestDir" )
	debugargs( "test_script.txt" )
	files(
	{
		"TestApp/src/**.h",
		"TestApp/src/**.cpp"
	} )
	filter( "configurations:Debug" )
		targetdir( "bin/Debug" )
		defines( "DEBUG" )
		symbols( "On" )
		fatalwarnings { "All" }