#-- Output character on the serial port: -------------------------------------

        .set    PORTCOM1, 0x3f8

        .text
        .globl	serial_putc
serial_putc:
        pushl   %eax
        pushl   %edx

        movw    $(PORTCOM1+5), %dx
1:      inb     %dx, %al        # Wait for port to be ready
        andb    $0x60, %al
        jz      1b

        movw    $PORTCOM1, %dx  # Output the character
        movb    12(%esp), %al
        outb    %al, %dx

        cmpb    $0xa, %al       # Was it a newline?
        jnz     2f

        movw    $(PORTCOM1+5), %dx
1:      inb     %dx, %al        # Wait again for port to be ready
        andb    $0x60, %al
        jz      1b

        movw    $PORTCOM1, %dx  # Send a carriage return
        movb    $0xd, %al
        outb    %al, %dx

2:      popl    %edx
        popl    %eax
        ret

#-- Done ---------------------------------------------------------------------
