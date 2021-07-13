# Cross Compile
Download compiler according to your enviroment as the below link:

https://github.com/sifive/freedom-tools/releases/tag/v2020.12.0

Add the compiler to your PATH:

export PATH=/home/user/compiler/bin:$PATH

# Build
Enter the project directory, generate the jh7100_recovery_boot.bin file after make in debug file

# Upgrade
1: Enter the debug mode

2: Uart baudrate is 9600,8N1

3: Enter "load 0x18000000" in terminal

4: Send the "debug/jh7100_recovery_boot.bin" with Xmodem

5: Enter "do 0x18000000" in terminal

If every is ok,the terminal will show as follow:

***************************************************

***************JH7100 recovery boot ***************

***************************************************

0:updata bootloader

1:updata ddr init

Select the function to test :

You can updata bootloader or ddr init
	
