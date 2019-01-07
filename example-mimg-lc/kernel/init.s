#-----------------------------------------------------------------------------
# init.s:  Simple kernel initialization code
#
# Mark P. Jones, March 2006, 2016

        .text
        .globl	entry
entry:	leal    stack, %esp             # Set up initial kernel stack
        call    kernel
1:      hlt                             # Halt CPU if kernel returns
        jmp     1b

        .data                           # Make space for a simple stack
	.align	4
        .space  4096
stack:

#-- Done ---------------------------------------------------------------------
