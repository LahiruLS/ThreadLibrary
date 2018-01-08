.text
.global machine_switch

machine_switch:
	# address of the new sp is arg1
	# address of the current tcb is arg2
	# need to store all required registered for old tcb
	# restore all required registred from the new tcb
	# then when you return, you should get to the new thread 
        # %rdi = New Thread first argument
        # %rsi = Old thread second argument 

#when switching between threads

# Saving the current running threads stack   
        
        # movq %rax, x(%rsi) No need for this lab cause every thread is void
      	#backingup  calee saved registers across function calls
        push %rbp;
        push %rbx;
	push %r12;
	push %r13;
	push %r14;
	push %r15;

	#update the stack pointer of previously ran thread
	mov %rsp,(%rsi);  
	
# Loading next threads stack(The stack we made) into OS's stack 
	#switching to the new threads stack
    	mov (%rdi),%rsp;

	#restore calee saved registers which are backuped earlierly from new stack
	pop %r15;
        pop %r14;
	pop %r13;
	pop %r12;
	pop %rbx;
	pop %rbp;
	    
	ret 

# starting of the first Thread
.INI:
    pushq %rdi
    ret
#No need for this found another way to steal main thread easily

#when running the last thread
.L1:
    ret 


