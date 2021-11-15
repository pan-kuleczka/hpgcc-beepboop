# hpgcc-beepboop
Project for playing .abc files on a HP 49/50 series calculators using the HPGCC library

## Cross compilation
1. Setup the hpgcc environment using the (unofficial) guide available here: 
   
    http://sense.net/~egan/hpgcc/
2. On the calculator create a directory for the executable and for the .abc files you intend to play. The preferable location is somewhere on the SD Card, as the total size of your music library may exceed the internal storage capacity
3. Compile the main.c file using the provided Makefile or the one supplied with hpgcc:
   
    `make main.hp`
4. Move the resulting .hp file and your .abc files to the calculator (via serial/USB or an SD Card)
5. (Optional) Convert the .hp file to a calculator executable using the ARM Toolbox

## Automatic execution using a UserRPL wrapper
This is the preferred method of running hpgcc-beepboop, as it is more user-friendly. Use the provided wrapper or create your own to suit your needs
1. Put the relative path to the .abc file on the stack
2. Execute the wrapper

## Manual execution
This method is not preferred, but maximizes your ability to move your executable
1. Put the relative path to the .abc file on the stack
2. Execute the program
3. Exit code (returned to the stack) of 0 indicates no errors, and a non-zero exit code will be supplied with an error message
