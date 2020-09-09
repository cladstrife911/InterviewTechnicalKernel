#How to build
Run 'make'

#How to build test application
'gcc -o test_app test_app.c'

#How to test
Build the Kernel driver and mount it to the Kernel:
'sudo insmod driver.ko'

Open the system log to check the events:
'tail -f /var/log/syslog'

#Environment
Tested on Ubuntu 20.04
