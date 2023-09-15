
.word 0x100 -2
.word 0x101 63
.word 0x102 -12
.word 0x103 54
.word 0x104 331
.word 0x105 -6
.word 0x106 2
.word 0x107 10
.word 0x108 1
.word 0x109 11
.word 0x10A -211
.word 0x10B 32
.word 0x10C 34
.word 0x10D -2
.word 0x10E 6
.word 0x10F -1234


		sub $sp, $sp,   $imm,   3			# clearing 3 spots at the stack
		sw  $s0, $sp,   $imm,   0			# storing $s0 at the stack (so we wont lose any information)
		sw  $s1, $sp,   $imm,   1 			# storing $s1 at the stack (so we wont lose any information)
		sw  $s2, $sp,   $imm,   2			# storing $s2 at the stack (so we wont lose any information)
		
		add $s0, $zero, $imm,   0x100		# loading the first element in array
		add $s1, $zero, $imm,   0x10E		# loading the last element of the array 

for_1:	add $s2, $s0, $zero,	0			# setting the second for loop index to 0.

for_2:  lw  $t0,  $s2, $zero,   0			# setting $t0=arr[$s2] (the first element we want to compare)
		add $t1,  $s2, $imm,	1			# saving the index $s2 + 1 into $t1 
		lw  $t2,  $t1, $zero,  	0			# setting $t2=arr[$t1] (the next element that we want to compare)
		ble $imm, $t0, $t2,  	skipped		# skipping the swap if the element is smaller or equal to the first one
		sw  $t2,  $s2, $zero,  	0			# swapping
		sw  $t0,  $t1, $zero,  	0			# swapping

skipped: add  $s2,   $s2,   $imm,  1			# advancing the second for loop index by 1
		ble  $imm,  $s2,   $s1,   for_2		# condition for the second loop
		sub  $s1,   $s1,   $imm,  1			# $s1--
		ble  $imm,  $s0,   $s1,   for_1	    # condition for the first loop
	
	
End:	lw   $s0,   $sp,   $imm,  0			# resroting the original value of $s0
		lw   $s1,   $sp,   $imm,  1			# resroting the original value of $s1
		lw   $s2,   $sp,   $imm,  2			# resroting the original value of $s2
		add  $sp,   $sp,   $imm,  3			# recover stack to its original form
		halt $zero, $zero, $zero, 0	        # end





