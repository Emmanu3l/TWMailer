WSL stores your Windows drives in the /mnt folder, with the name of the drive as a subfolder.
For example your C:\ drive will be present at /mnt/c/ for you to use.

call server like this:
./myserver.o directoryName

and telnet like this:
telnet localhost portNumber


in conclusion:
cd /mnt/c/wsl_twmailer/
make
./myserver.o directoryName
telnet localhost 6644 [in separate window]