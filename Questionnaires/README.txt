Project description:
We completed lab 6 with EEPROM MAC and "zero copy" challenges. In addition we created VGA and
mouse drivers.
In the VGA driver we initiated vga screen with resulotion 320X200 with 256 colors and added kernel
api to draw different shapes on the screen. In addition we added support to write letters on the 
screen. This api is exposed to the user with syscalls and the user is able to draw different shapes
with different colors on the screen and print complete sentences to the screen.
To show the user abilities we created 2 games: Ping Pong and Tic-Tac-Toe to show vga capabilities.

In the mouse driver, we implemented PS/2 mouse that uses polling and interrupts to send messages
to the kernel about movement and clicking. The mouse supports left click, right click and double click
and can be used in different applications. We used a pool that stores mouse packets that there was a
click in them and we expose the pool to the user using syscall (exactly how we pool from the keyboard
or console), the user gets a struct contains the current location of the mouse and the click type and
can use it for his applications. We added user test called testmouse to show mouse capabilities.

All the tests we included can be run as stand alone environment and or through the file system with
the name of the file like any other user code running through the file system.
