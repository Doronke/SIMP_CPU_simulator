	add $sp, $sp, $imm, -7						# adjust space in stack for 7 items
	sw $s0, $sp, $imm, 6						# store $s0 in the stack
	sw $s1, $sp, $imm, 5						# store $s1 in the stack
	sw $s2, $sp, $imm, 4						# store $s2 in the stack
	sw $a0, $sp, $imm, 3						# store $a0 in the stack
	sw $a1, $sp, $imm, 2						# store $a1 in the stack
	sw $v0, $sp, $imm, 1						# store $v0 in the stack
	sw $ra,	$sp, $imm, 0						# save return address in the stack
	add $t0, $zero, $zero, 0					# initialize $t0=0  
	add $t1, $zero, $zero, 0					# initialize $t1=0
	add $t2, $zero, $imm, 1023					# set $t2=1023
 	add $t3, $zero, $imm, HANDLER				# $t3 holds HANDLER address
	out $t3, $zero, $imm, 6						# IOregister[6] = HANDLER address
	out $t2, $zero, $imm, 13					# set IOregister[13] , timermax ,to $t2 = 1023.
	add $s0, $zero, $imm, 1						# set $s0=1
	add $s1, $zero, $zero, 0					# initialize $s0=0
	add $t3, $zero, $zero, 0					# initialize $t3=0
 	add $t2, $zero, $zero, 0					# initialize $t2=0	
	
	out $s0, $zero,  $imm , 0					# set irq0enable = 1
	out $s0, $zero,  $imm , 1					# set irq1enable = 1
	out $s0, $zero,  $imm , 2					# set irq2enable = 1
	out $s0, $zero,  $imm, 11  					# set timerenable=1 - starting the timer

FOREVER_LOOP:	
	out $v0, $zero, $imm, 10					# set $v0 = IORegister [10] , register 10 is display7seg
	beq $imm, $zero, $zero, FOREVER_LOOP		# jump always to FOREVER_LOOP	     	

HANDLER:
	in $a1,  $zero,  $imm, 3					# check the value in register irq0status and set $a1 = irq0status
	in $s2,  $zero,  $imm, 4					# check the value in register irq1status and set $s2 = irq1status 
	in $a0,  $zero,  $imm, 5					# check the value in register irq2status and set $a0 = irq2status 
	
	beq $imm, $s0, $s2, PAUSE					# if $s2 = 1 go to PAUSE
	beq $imm, $s0, $a1, STOPER1					# if $a1 = 1 go to STOPER1
	beq $imm, $s0, $a0, INITIALIZE1				# if $a0 = 1 go to INITIALIZE1

STOPER1: 	
	out $zero, $zero,  $imm, 3					# initialize irq0status = 0
	out $zero, $zero,  $imm, 1 					# initialize irq1enable = 0
	out $zero, $zero,  $imm, 2					# initialize irq2enable = 0
	add $t0, $t0, $imm, 1						# timer += 1sec
	add $s1, $zero, $zero, 0					# initialize $s1 = 0 
	add $s1, $zero, $imm, 10					# set $s1 = 10
	beq $imm, $t0, $s1, STOPER2					# if the time is XX:XA go to STOPER2
	add $v0 , $v0 , $imm , 1					# $v0 = $v0 +1
	out $s0, $zero,  $imm, 1 					# set irq1enable = 1
	out $s0, $zero,  $imm, 2					# set irq2enable = 1
	reti $zero, $zero, $zero, 0					# go back to original PC
	

STOPER2:
	add $t0, $zero, $zero, 0					# initialize $t0 = 0 
	add $s1, $zero, $zero, 0					# initialize $s1 = 0 
	add $s1, $zero, $imm, 6						# set $s1 = 6 
	add $t1, $t1, $imm, 1						# $t1 += 1
	beq $imm, $t1, $s1, STOPER3					# if the time is XX:6X go to STOPER3
	add $v0, $v0, $imm, 7						# $v0 = $v0 + 7
	out $s0, $zero,  $imm, 1 					# set irq1enable =1
	out $s0, $zero,  $imm, 2					# set irq2enable =1
	reti $zero, $zero, $zero, 0					# go back to original PC
	

STOPER3:
	add $t0, $zero, $zero, 0					# initialize $t0 = 0 
	add $t1, $zero, $zero, 0					# initialize $t1 = 0 
	add $s1, $zero, $zero, 0					# initialize $s1 = 0 
	add $s1, $zero, $imm, 10					# set s1 = 10
	add $t2, $t2, $imm, 1						# $t2+=1
	beq $imm, $t2, $s1, STOPER4					# if the time is XA:XX go to STOPER4
	add $v0, $v0, $imm, 167						# $v0 = $v0 + 167
	out $s0, $zero,	$imm, 1 					# set irq1enable =1
	out $s0, $zero,	$imm, 2						# set irq2enable =1
	reti $zero, $zero, $zero, 0					# go back to original PC

STOPER4:
	add $t0, $zero, $zero, 0					# initialize $t0 = 0  
	add $t1, $zero, $zero, 0					# initialize $t1 = 0
	add $t2, $zero, $zero, 0					# intialilze $t2 = 0 
	add $s1, $zero, $zero, 0					# intialilze $s1 = 0 
	add $s1, $zero, $imm, 6						# set $s1 = 6
	add $t3, $t3, $imm, 1						# $t3+=1	 
	beq $imm, $t3, $s1, INITIALIZE1				# if the time is 6X:XX go to INITIALIZE1
	add $v0, $v0, $imm, 1703					# $v0 = $v0 + 1703
	out $s0, $zero,  $imm, 1 					# set irq1enable =1
	out $s0, $zero,  $imm, 2					# set irq2enable =1
	reti $zero, $zero, $zero, 0					# go back to original PC


PAUSE:
	out $zero, $zero, $imm, 4				# initialize irqstatus1 = 0
	in $s1, $zero, $imm, 11					# check the value in register 11 and put in $s1 = IOregister[11]
	sub $s1, $s0, $s1, 0					# $s1 = 1 - $s1
	out $s1, $zero, $imm, 11				# set register timerenable
	add $s1, $zero, $imm, 0					# initialize $s1 = 0 
	reti $zero, $zero, $zero, 0				# go back to original PC
	

INITIALIZE1:
	out $zero, $zero,  $imm, 5			# initialize irq2status = 0
	out $zero, $zero,  $imm, 3			# initialize irq0status = 0
	add $t0, $zero, $zero, 0			# intialilze $t0 = 0 
	add $t1, $zero, $zero, 0 			# intialilze $t1 = 0 
	add $t2, $zero, $zero, 0			# intialilze $t2 = 0 	
	add $t3, $zero, $zero, 0			# intialilze $t3 = 0 
	add $v0, $zero, $zero, 0			# intialilze $v0 = 0 
	out $v0, $zero, $imm, 10			# $v0 = IORegister [10], register 10 is display7seg				
	out $s0, $zero,  $imm , 0     		# set irqenable0 = 1
	out $s0, $zero,  $imm , 1			# set irqenable1 = 1
	out $s0, $zero,	$imm , 2			# set irqenable2 = 1
	reti $zero, $zero, $zero, 0			# go back to original PC
	
					
