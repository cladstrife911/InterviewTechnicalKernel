#How to build
From src folder, run 'make'

#How to build test application
From src folder, 'gcc -o test_app test_app.c'

#How to test
From src folder, build the Kernel driver and mount it to the Kernel:
'sudo insmod driver.ko'

From another terminal window, open the system log to check the events:
'tail -f /var/log/syslog'

From another terminal window, run a ping request (CMP) either:
	'ping localhost'
or
	'ping www.google.com'

From src folder, build and run the test application with:
'sudo ./src/test_app'

#Environment
Tested on Ubuntu 20.04
