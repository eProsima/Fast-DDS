If this test has been build using colcon then remember to properly setup the environmental variables before running the binary by using:

Linux:
		$ . ../../../../../local_setup.bash
		
Windows:

		>..\..\..\..\..\local_setup.bat
		
otherwise modify the console PATH or terminal LIB_PATH_DIR environmental variables to allow the example binary to locate fast shared libraries.

Run the example creating three independent processes:

Linux:

	Terminal 1:
	
		$ ./HelloWorldExampleDS publisher
		
	Terminal 2:
	
		$ ./HelloWorldExampleDS subscriber	

	Terminal 3:
	
		$ ./HelloWorldExampleDS server	

Windows:
	
	Console 1:
	
		> HelloWorldExampleDS publisher
	
	Console 2:
	
		> HelloWorldExampleDS subscriber
	
	Console 3:
	
		> HelloWorldExampleDS server
		
Information exchange between publisher and subscriber should take place whenever the server begins running.

Usage: HelloWorldExampleDS <publisher|subscriber|server>

General options:
  -h        --help            Produce help message.
  -t        --tcp             Use tcp transport instead of the default UDP one.
  -c <num>, --count=<num>     Number of datagrams to send (0 = infinite)
                              defaults to 10.
  -i <num>, --interval=<num>  Time between samples in milliseconds (Default:
                              100).







