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


#references
- https://embetronicx.com/tutorials/linux/device-drivers/ioctl-tutorial-in-linux/

Netfilter:
- https://medium.com/bugbountywriteup/linux-kernel-communication-part-1-netfilter-hooks-15c07a5a5c4e

IOCTL:
- https://tldp.org/LDP/lkmpg/2.4/html/x856.html
