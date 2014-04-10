%ifndef BASE_YASM
%define BASE_YASM

; minimum stable AVX version it looks like
%if (__YASM_VERSION_ID__ < 00080000h) ; 0.8.0
	%error Requires Yasm 0.8.0 or higher
%endif

%define HAVE_XOP 0
%define HAVE_AVX2 0
%define HAVE_AVX512 0

%if (__YASM_VERSION_ID__ >= 01000000h) ; 1.0.0
	%define HAVE_XOP 1
%endif

%if (__YASM_VERSION_ID__ >= 01020000h) ; 1.2.0
	%define HAVE_AVX2 1
%endif


%define BITS32 0
%define BITS64 0
%define WIN 0
%define ELF 0
%define MACH 0

%ifidn __YASM_OBJFMT__, win32
	%define BITS32 1
	%define WIN 1
%elifidn __YASM_OBJFMT__, elf
	%error Specify bits with -f [elf32,elf64]
%elifidn __YASM_OBJFMT__, elf32
	%define BITS32 1
	%define ELF 1
%elifidn __YASM_OBJFMT__, macho
	%error Specify bits with -f [macho32,macho64]
%elifidn __YASM_OBJFMT__, macho32
	%define BITS32 1
	%define MACH 1
%elifidn __YASM_OBJFMT__, win64
	%define BITS64 1
	%define WIN 1
%elifidn __YASM_OBJFMT__, x64
	%define BITS64 1
	%define WIN 1
%elifidn __YASM_OBJFMT__, elf64
	%define BITS64 1
	%define ELF 1
%elifidn __YASM_OBJFMT__, macho64
	%define BITS64 1
	%define MACH 1
%else
	%error "Unable to determine output format"
%endif

%if (WIN)
	%if (BITS64)
		; name, args, xmmused
		%macro win64stubfn 3
			%1:
			_ %+ %1:

			subq $184, %rsp
			movdqa %xmm6, 0(%rsp)
			movdqa %xmm7, 16(%rsp)
			%if (%3 > 8)
				movdqa %xmm8, 32(%rsp)
				movdqa %xmm9, 48(%rsp)
				movdqa %xmm10, 64(%rsp)
				movdqa %xmm11, 80(%rsp)
				movdqa %xmm12, 96(%rsp)
				movdqa %xmm13, 112(%rsp)
				movdqa %xmm14, 128(%rsp)
				movdqa %xmm15, 144(%rsp)
			%endif
			movq %rdi, 160(%rsp)
			movq %rsi, 168(%rsp)
			movq %rcx, %rdi
			movq %rdx, %rsi
			movq %r8, %rdx
			movq %r9, %rcx
			%if (%2 >= 5)
				movq 224(%rsp), %r8
			%endif
			%if (%2 >= 6)
				movq 232(%rsp), %r9
			%endif
			call thunk_ %+ %1
			movdqa 0(%rsp), %xmm6
			movdqa 16(%rsp), %xmm7
			%if (%3 > 8)
				movdqa 32(%rsp), %xmm8
				movdqa 48(%rsp), %xmm9
				movdqa 64(%rsp), %xmm10
				movdqa 80(%rsp), %xmm11
				movdqa 96(%rsp), %xmm12
				movdqa 112(%rsp), %xmm13
				movdqa 128(%rsp), %xmm14
				movdqa 144(%rsp), %xmm15
			%endif
			movq 160(%rsp), %rdi
			movq 168(%rsp), %rsi
			addq $184, %rsp
			ret
		thunk_ %+ %1:
		%endmacro

		; FN name
		%macro FN 1
			win64stubfn %1, 4, 16
		%endmacro

		; FN_EXT name, args, xmmused
		%macro FN_EXT 1
			win64stubfn %1, %2, %3
		%endmacro

		; FN_END name
		%macro FN_END 1
		%endmacro
	%else
		; FN name
		%macro FN 1
			%1:
			_ %+ %1:
		%endmacro

		; FN_EXT name, args, xmmused
		%macro FN_EXT 3
			%1:
			_ %+ %1:
		%endmacro

		; FN_END name
		%macro FN_END 1
		%endmacro
	%endif
%elif (ELF)
	; FN name
	%macro FN 1
		%1:
		_ %+ %1:
	%endmacro

	; FN_EXT name, args, xmmused
	%macro FN_EXT 3
		%1:
		_ %+ %1:
	%endmacro

	; FN_END name
	%macro FN_END 1
		.size %1, .-%1
		.type %1, @function
	%endmacro

	; set NX for stack
	.section .note.GNU-stack,"",@progbits
%elif (MACH)
	; FN name
	%macro FN 1
		%1:
		_ %+ %1:
	%endmacro

	; FN_EXT name, args, xmmused
	%macro FN_EXT 3
		%1:
		_ %+ %1:
	%endmacro

	; FN_END name
	%macro FN_END 1
	%endmacro
%endif

; put everything in the code segment to simplify things
%define SECTION_TEXT .section .text
%define SECTION_RODATA .section .text

; declares a global function: GLOBAL name
%macro GLOBAL 1
	.globl %1
	.globl _ %+ %1
%endmacro

; pic support: LOAD_VAR_PIC var, reg
%macro LOAD_VAR_PIC 2
	%if (BITS32)
		call 1f
		1:
		popl %2
		leal %1 - 1b(%2), %2
	%else
		leaq %1(%rip), %2
	%endif
%endmacro

%macro INCLUDE 1
	%include %1
%endmacro

; include the file with the variable(s) if variable 'name' is not already included: INCLUDE_VAR_FILE file, name
%macro INCLUDE_VAR_FILE 2
	%ifndef INCLUDED_%2
		%define INCLUDED_%2
		%include %1
	%endif
%endmacro

%macro INCLUDE_IF_32BIT 1
	%if (BITS32)
		INCLUDE %1
	%endif
%endmacro

%macro INCLUDE_IF_64BIT 1
	%if (BITS64)
		INCLUDE %1
	%endif
%endmacro


%macro INCLUDE_IF_MMX_32BIT 1
	INCLUDE_IF_32BIT %1
%endmacro

%macro INCLUDE_IF_MMX_64BIT 1
	INCLUDE_IF_64BIT %1
%endmacro


%macro INCLUDE_IF_SSE_32BIT 1
	INCLUDE_IF_32BIT %1
%endmacro

%macro INCLUDE_IF_SSE_64BIT 1
	INCLUDE_IF_64BIT %1
%endmacro


%macro INCLUDE_IF_SSE2_32BIT 1
	INCLUDE_IF_32BIT %1
%endmacro

%macro INCLUDE_IF_SSE2_64BIT 1
	INCLUDE_IF_64BIT %1
%endmacro


%macro INCLUDE_IF_SSE3_32BIT 1
	INCLUDE_IF_32BIT %1
%endmacro

%macro INCLUDE_IF_SSE3_64BIT 1
	INCLUDE_IF_64BIT %1
%endmacro


%macro INCLUDE_IF_SSSE3_32BIT 1
	INCLUDE_IF_32BIT %1
%endmacro

%macro INCLUDE_IF_SSSE3_64BIT 1
	INCLUDE_IF_64BIT %1
%endmacro


%macro INCLUDE_IF_SSE4_1_32BIT 1
	INCLUDE_IF_32BIT %1
%endmacro

%macro INCLUDE_IF_SSE4_1_64BIT 1
	INCLUDE_IF_64BIT %1
%endmacro


%macro INCLUDE_IF_SSE4_2_32BIT 1
	INCLUDE_IF_32BIT %1
%endmacro

%macro INCLUDE_IF_SSE4_2_64BIT 1
	INCLUDE_IF_64BIT %1
%endmacro


%macro INCLUDE_IF_AVX_32BIT 1
	INCLUDE_IF_32BIT %1
%endmacro

%macro INCLUDE_IF_AVX_64BIT 1
	INCLUDE_IF_64BIT %1
%endmacro


%macro INCLUDE_IF_XOP_32BIT 1
	%if HAVE_XOP
		INCLUDE_IF_32BIT %1
	%endif
%endmacro

%macro INCLUDE_IF_XOP_64BIT 1
	%if HAVE_XOP
		INCLUDE_IF_64BIT %1
	%endif
%endmacro


%macro INCLUDE_IF_AVX2_32BIT 1
	%if HAVE_AVX2
		INCLUDE_IF_32BIT %1
	%endif
%endmacro

%macro INCLUDE_IF_AVX2_64BIT 1
	%if HAVE_AVX2
		INCLUDE_IF_64BIT %1
	%endif
%endmacro


%macro INCLUDE_IF_AVX512_32BIT 1
	%if HAVE_AVX512
		INCLUDE_IF_32BIT %1
	%endif
%endmacro

%macro INCLUDE_IF_AVX512_64BIT 1
	%if HAVE_AVX512
		INCLUDE_IF_64BIT %1
	%endif
%endmacro


/* stupid helpers so we can have cpuid in one file */

%macro CPUID_PROLOGUE 0
%if (BITS32)
	pushl %ebx
	pushl %esi
	pushl %edi
	pushl %ebp

	/* check that cpuid is supported */
	pushfl
	popl %eax
	movl %eax, %ecx
	xorl $0x200000, %eax
	pushl %eax
	popfl
	pushfl
	popl %eax
	xorl %ecx, %eax
	shrl $21, %eax
	andl $1, %eax
	pushl %ecx
	popfl
	andl %eax, %eax
	jz cpuid_x86_done
%else
	pushq %rbx
	pushq %rsi
	pushq %rdi
	pushq %rbp
%endif
%endmacro

%macro CPUID_EPILOGUE 0
%if (BITS32)
	popl %ebp
	popl %edi
	popl %esi
	popl %ebx
%else
	popq %rbp
	popq %rdi
	popq %rsi
	popq %rbx
%endif
	ret
%endmacro

%endif ; BASE_YASM