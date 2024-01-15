
; @file callDispatcher.x64.asm
; @author Alejandro González (@httpyxel)
; @brief Assembly utilities  to dispatch windows native calls.
; @version 0.1
; @date 2024-01-14
;
; @copyright 
;   Copyright (C) 2024  Alejandro González
;
;   This program is free software: you can redistribute it and/or modify
;   it under the terms of the GNU General Public License as published by
;   the Free Software Foundation, either version 3 of the License, or
;   (at your option) any later version.
;
;   This program is distributed in the hope that it will be useful,
;   but WITHOUT ANY WARRANTY; without even the implied warranty of
;   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;   GNU General Public License for more details.
;
;   You should have received a copy of the GNU General Public License
;   along with this program.  If not, see <https://www.gnu.org/licenses/>.

%include "callDispatcher/asm/callDispatcher.x64.hasm"

ENFORCE_FORMAT win64
[BITS 64]

;void* __callobf_doCall(void* p_function, unsigned short ssn, unsigned long isSyscall, unsigned lon argCount, void* p_args, p_returnAddress, void* p_globalFramTable)
__callobf_doCall:
    set_opt stored_regs             , rbp, rsi, rdi
    set_opt arg_count               , 6
    set_opt local_var_space         , 0x10
    set_opt max_callee_arg_count    , 1
    set_opt store_args              , true
    
    end_opts

; ===================== Define args =======================

    %define pp_function         stored_arg(0)
    %define p_ssn               stored_arg(1)
    %define p_isSyscall         stored_arg(2)
    %define p_argCount          stored_arg(3)
    %define pp_args             stored_arg(4)
    %define pp_returnAddress    stored_arg(5)
    %define pp_globalFrameTable stored_arg(6)

    %define pp_newRsp           local_var(0)
    %define pp_restore          local_var(1)

; ======================= Code ============================
    mov rbp, rsp

    fastcall_win64 __callobf_buildSpoofedCallStack, [pp_globalFrameTable]
    checkNull rax, error

    mov [pp_newRsp], rax

;   Store reference to restore subroutine
    lea rax, [rel restore]
    mov [pp_restore], rax

;   Move arguments 4+
    mov rcx, [p_argCount]
    sub rcx, 4
    jle lessThanFiveArgs

        mov rsi, [pp_args]
        add rsi, 0x20

        mov rdi, [pp_newRsp]
        add rdi, 0x28

        rep movsq

    lessThanFiveArgs:

;   Move arguments 0-3
    mov rsi, [pp_args]
    mov rcx, [rsi]
    mov rdx, [rsi + 0x8]
    mov r8,  [rsi + 0x10]
    mov r9,  [rsi + 0x18]

    mov r10, [stored_arg(2)]

;   Set ssn and first arg if syscall
    checkNull r10, notSyscall
        mov r10, rcx
        mov rax, [stored_arg(1)]
    notSyscall:

; Set rbx to point to the reference to restore
    lea rbx, [pp_restore]

; Take any variable we need from the stack before changing rsp
    mov r11, [pp_function]

    mov rsp, [pp_newRsp]
    jmp r11

restore:
    mov rsp, rbp
    end_function

error:
    xor rax, rax

    mov rsp, rbp
    end_function

