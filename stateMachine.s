.data

swone_counter: .word 0 ; swone_counter =1 

.text

jumptable:
			.word case0 ;jumptable[0]
			.word case1 ; jumptable[1]
			.word case2 ; jumptable[2]
			.word case3 ; jumptable[3]
			.word case4 ; jumptable[4]
			.word case5 ; jumptable[5]

			.global switch_one
switch_one:
			mov &swone_counter , r12 ; move swonecounter to r12
			add r12 , r12 ; r12 
			mov jumptable(r12) , r0

case0:
		mov #40, r12
		CALL #buzzer_set_period
		add #1 , &swone_counter
		
		JMP break


case1:
		mov #300 , r12
		CALL #buzzer_set_period
		add #1 , &swone_counter
		JMP break


case2:
		mov #100 , r12
		CALL #buzzer_set_period
		add #1 , &swone_counter
		JMP break

case3:
		mov #800 , r12
		CALL #buzzer_set_period
		add #1 , &swone_counter
		JMP break 
case4:
		mov #50 , r12
		CALL #buzzer_set_period
		add #1 , &swone_counter
		JMP break

case5:
		mov #900 , r12
		CALL #buzzer_set_period
		mov #0 , &swone_counter
		JMP break


break: 
		ret

