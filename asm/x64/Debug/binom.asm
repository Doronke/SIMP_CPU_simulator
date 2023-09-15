.word 0x100 8
.word 0x101 3	
	
	lw $a0, $imm, $zero, 0x100		#Set the reg a0 to be the original value of n
	lw $a1, $imm, $zero, 0x101		#Set the reg a1 to be the original value of k
	jal $imm, $zero, $zero, BINOM	#Go to BINOM
	sw $v0, $zero, $imm, 0x102		#Store the final value in $v0
	halt $zero, $zero, $zero, 0		#Halt	

BINOM:	
	sub $sp, $sp, $imm, 4			#Allocate space in stack for registers
	sw  $a0,  $imm,  $sp,   3		# Save n in the stack
	sw  $a1,  $imm,  $sp,   2		# Save k in the stack
	sw $s0, $imm, $sp, 1			# Save $s0 to the stack
	sw $ra, $imm, $sp, 0			# Save $ra to the stack

	beq $imm, $a1, $zero, RETURN	# If k == 0 then go to RETURN
	beq $imm, $a0, $a1, RETURN		# If n == k then go to RETURN		
	bgt $imm, $a0, $a1, LOOP		# If n > k then go to LOOP1

	
LOOP:
	bgt $imm, $a1, $zero, MAINLOOP	# If k > 0 then go to MAINLOOP
	beq $imm, $a0, $zero, RETURN	# If k == 0 then go to RETURN

RETURN:
	add $v0, $v0, $imm, 1			# Else, BIN(n,k: n=0 or k=0) == 1
	beq $imm, $zero, $zero, END		# Go to return loop


MAINLOOP:	
		sub $a0, $a0, $imm, 1, 						# n --> n - 1
		sub $a1, $a1, $imm, 1						# k --> k - 1
		jal $imm, $zero, $zero, BINOM				# BIN(n-1, k-1)
		add $s0, $v0, $imm, $zero					# Save BIN(n-1, k-1) to reg $s0 

		add $a1, $a1, $imm, 1						# k <-- k - 1
		jal $imm, $zero, $zero, BINOM				# BIN(n-1, k)
		add $v0, $v0, $imm, $s0						# v0 = bin(n-1, k-1) + bin(n-1, k)

END:	
		lw $a0, $sp, $imm, 3				# restore n in the stack
		lw $a1, $sp, $imm, 2				# restore k in the stack
		lw $s0, $sp, $imm, 1				# Restore $ra from the stack
		lw $ra, $sp, $imm, 0				# Restore $s0 from the stack

		add $sp, $sp, $imm, 4				#Release allocated space from the stack
		beq $ra, $zero, $zero, 0			#Return to caller

