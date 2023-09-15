# SIMP CPU simulator
SIMP CPU simulator and assembler built in C++. 2 version are available PC version and BASYS chip version.
full documentation added in hebrew as PDF and explenation for the testing assembly progrems written in MIPS assembly the documentation of the code is written in english.

## PC simulation

The simulation recieve memin.txt file, created by the assembler and return txt file containing memory, registers ,number of cycles at the end of the run and th trace.

![image](https://github.com/Doronke/SIMP_CPU_simulator/assets/131671196/aa682c87-d1ba-448c-97d0-dce89b6359ce)

the registers simulated:

![image](https://github.com/Doronke/SIMP_CPU_simulator/assets/131671196/7883c00d-b402-4e53-93e3-d777a8ed5f7e)

commands structure:

![image](https://github.com/Doronke/SIMP_CPU_simulator/assets/131671196/76862f69-17b3-4704-a85e-3584d4615410)

commands simulated:

![image](https://github.com/Doronke/SIMP_CPU_simulator/assets/131671196/c8aa11d9-9594-4ac4-8f6b-05f1c63e106b)

test progrems:

fib.asm - calculate fibonacci number.
bubble.asm - sort a series of numbers using bubble sort algorithm.
binom.asm - calculate the binom coefficent using recursion.  

## BASYS simulation

built on the PC simulation where there's I/O registers added, and command to speak/write.
register/cycle/memory information can be seen on the BASYS display using the switch 0,1 buttons.
BTNL used to pause the simulation , while in pause BTNR will run one command line, another BTNL press will unpausee the simulation.

![image](https://github.com/Doronke/SIMP_CPU_simulator/assets/131671196/3cba2e74-913c-4334-9ecd-534451e47f14)

![image](https://github.com/Doronke/SIMP_CPU_simulator/assets/131671196/efc12080-83ac-4b7c-bd97-f8cef4a7c0cc)

![image](https://github.com/Doronke/SIMP_CPU_simulator/assets/131671196/a8d6adf6-4ed3-4e8d-9f6f-37b93ae1d4b6)

in addition irq0, irq1, irq2 commands were added ,basic LED control, and button control,explained in details in the PDF documentation file.

the test progrem Stopper.asm is a timer that run on the BASYS 7 segment display , with PAUSE/UNPAUSE and RESET options using BTNC , BTNL. the timer counts up to 59[min]:59[sec] and then resets to 00:00.
full explemantion of each button function is in the PDF documentation.

