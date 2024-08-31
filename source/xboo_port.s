
//-----------------------------------------------------------------------------
	.bss
//-----------------------------------------------------------------------------
	.comm	PortAddress,2
	.comm	CRC1,2
	.comm	CRC2,2

//-----------------------------------------------------------------------------
	.data
//-----------------------------------------------------------------------------
parport_byte:
	.value 4

//-----------------------------------------------------------------------------
	.text
//-----------------------------------------------------------------------------
	.global _SetPortAddress
	.global SetPortAddress
//-----------------------------------------------------------------------------
SetPortAddress:
_SetPortAddress:
//-----------------------------------------------------------------------------
	movl	4(%esp),%eax
	andb	$0xFC,%al		/* ensure base address is used */
	movw	%ax,PortAddress
	ret

//-----------------------------------------------------------------------------
	.global _GetPortAddress
	.global GetPortAddress
//-----------------------------------------------------------------------------
GetPortAddress:
_GetPortAddress:
//-----------------------------------------------------------------------------
	movswl	PortAddress,%eax
	ret

//-----------------------------------------------------------------------------
	.global _InitPort
	.global InitPort
//-----------------------------------------------------------------------------
InitPort:
_InitPort:
//-----------------------------------------------------------------------------
	movw	PortAddress,%dx
	incw	%dx			/* move to control port		*/
	incw	%dx
	movb	parport_byte,%al
	out	%al, %dx
	andb	$0xFC,%dl
	movb	$0xFF,%al
	out	%al,%dx
	ret

//-----------------------------------------------------------------------------
	.global _ResetGBA
	.global ResetGBA
//-----------------------------------------------------------------------------
ResetGBA:
_ResetGBA:
//-----------------------------------------------------------------------------
	push	%ecx
	movw	PortAddress,%dx
	movb	parport_byte,%ah
	and	$0xFB,%ah
	mov	%ah, %al
	out	%al, %dx
	xorl	%ecx,%ecx
	mov	$4,%cx
hold:
	push	%ax
	in	%dx, %al
	in	%dx, %al
	in	%dx, %al
	in	%dx, %al
	in	%dx, %al
	in	%dx, %al
	in	%dx, %al
	in	%dx, %al
	in	%dx, %al
	in	%dx, %al
	in	%dx, %al
	in	%dx, %al
	pop	%ax
	loop	hold

	or	$4,%ah
	mov	%ah, %al
	out	%al,%dx
	pop	%ecx
	ret

//-----------------------------------------------------------------------------
	.global _XbooSend32
	.global XbooSend32
//-----------------------------------------------------------------------------
XbooSend32:
_XbooSend32:
//-----------------------------------------------------------------------------
	push	%ebx
	push	%ecx
	mov	12(%esp),%ebx
	movw	PortAddress,%dx
	incw	%dx			/* move to control port		*/
	incw	%dx
	xor	%ecx,%ecx
	mov	$32,%cx			/* 32 bits to send	*/
.NxtBit:
	shl	$1,%ebx
	sbb	%al,%al
	and	$2,%al
	xor	$7,%al
	out	%al,%dx
	dec	%ax
	out	%al,%dx
	and	$0xFD,%al
	out	%al,%dx
	loop	.NxtBit
	pop	%ecx
	pop	%ebx
	ret


//-----------------------------------------------------------------------------
	.global _XbooExchange32
	.global XbooExchange32
//-----------------------------------------------------------------------------
XbooExchange32:
_XbooExchange32:
//-----------------------------------------------------------------------------
	push	%ebx
	push	%ecx
	mov	12(%esp),%ebx
	movw	PortAddress,%dx
	incw	%dx			/* move to control port		*/
	incw	%dx
	movb	parport_byte,%ah
	xor	%ecx,%ecx
	mov	$32,%cx			/* 32 bits to send		*/
//-----------------------------------------------------------------------------
.NextBit:
//-----------------------------------------------------------------------------
	shl	$1,%ebx			/* get next bit to send		*/
	sbb	%al,%al			/* al=bit?0xff:0x00		*/
	andw	$0x0c02,%ax		/* al=bit?0x02:0x00		*/
					/* retain SD state, Reset	*/
					/* clear SC, set SI 		*/
	xorb	$3,%al			/* invert bit & SC low		*/
	xorb	%ah, %al		/* saved SD state & Reset	*/
	outb	%al,%dx			/* write to port		*/
	movb	%al, %ah		/* preserve data written	*/
	decw	%dx			/* move to status port		*/
	inb	%dx,%al			/* read from port		*/
	incw	%dx			/* back to control port		*/
	shlb	$2,%al			/* bit 6 to carry (GBA SO)	*/
	adcw	$0,%bx			/* ebx+=inbit			*/
	movb	%ah, %al		/* restore data			*/
	decw	%ax			/* SC high			*/
	outb	%al,%dx			/* write to port		*/
	andb	$0xFD,%al		/* clear bit 1 (GBA SI high)	*/
	outb	%al,%dx			/* write to port		*/
	loop	.NextBit
	movb	%al,parport_byte
	movl	%ebx,%eax
	pop	%ecx
	pop	%ebx
	ret

//-----------------------------------------------------------------------------
	.global _PortDelay
	.global PortDelay
//-----------------------------------------------------------------------------
PortDelay:
_PortDelay:
//-----------------------------------------------------------------------------
	push	%ecx
	movw	PortAddress,%dx
	mov	8(%esp),%ecx
	incl	%ecx
.wait:
	in	%dx,%al
	in	%dx,%al
	in	%dx,%al
	in	%dx,%al
	in	%dx,%al
	in	%dx,%al
	loop	.wait
	pop	%ecx
	ret

//-----------------------------------------------------------------------------
	.global	CalcCRC
	.global	_CalcCRC
//-----------------------------------------------------------------------------
CalcCRC:
_CalcCRC:
//-----------------------------------------------------------------------------
	push	%ecx
	mov	8(%esp),%eax
	movw	CRC2,%dx
	xorw	CRC1,%ax
	xorl	%ecx,%ecx
	mov	$32,%cx
crc:
	shrl	$1,%eax
	jnb	skip
	xorw	%dx,%ax
skip:
	loop	crc
	movw	%ax,CRC1
	pop	%ecx
	ret
//-----------------------------------------------------------------------------
	.global	InitCRC
	.global	_InitCRC
//-----------------------------------------------------------------------------
InitCRC:
_InitCRC:
//-----------------------------------------------------------------------------
	movw	$0x0C387,CRC1
	movw	$0x0C37B,CRC2
	ret


//-----------------------------------------------------------------------------
	.global	ReadSOState
	.global	_ReadSOState
//-----------------------------------------------------------------------------
ReadSOState:
_ReadSOState:
//-----------------------------------------------------------------------------
	pushl	%ebx
	xor	%ebx,%ebx
	movw	PortAddress,%dx
	incw	%dx			/* move to status port		*/
	inb	%dx,%al			/* read from port		*/
	shlb	$2,%al			/* bit 6 to carry (GBA SO)	*/
	adcw	$0,%bx			/* ebx+=inbit			*/
	in	%dx,%al			/* delay to make sure GBA has started comms */
	in	%dx,%al
	in	%dx,%al
	in	%dx,%al
	in	%dx,%al
	in	%dx,%al
	in	%dx,%al
	movl	%ebx,%eax
	popl	%ebx
	ret
