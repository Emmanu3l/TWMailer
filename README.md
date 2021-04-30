WSL stores your Windows drives in the /mnt folder, with the name of the drive as a subfolder.

For example your C:\ drive will be present at /mnt/c/ for you to use.


* Call the server like this:

`./myserver.o directoryName`


* And telnet like this:

`telnet localhost portNumber`


## In conclusion:

* In one terminal:

`cd /mnt/c/wsl_twmailer/`

`make`


* In the other terminal:

`./myserver.o directoryName`

`telnet localhost 6655`