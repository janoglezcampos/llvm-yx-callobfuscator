; @file stackSpoofHelper.x64.asm
; @author Alejandro González (@httpyxel)
; @brief Assembly utilities to build spoofed stacks for windows x64 native function.
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

%include "stackSpoof/asm/stackSpoofHelpers.x64.hasm"

ENFORCE_FORMAT win64
[BITS 64]

; Imported functions
EXTERN __callobf_lfsrXorShift32
EXTERN __callobf_initializeSpoofInfo

; Exported functions
GLOBAL __callobf_buildSpoofedCallStack

[SECTION .text]

; This function should only be used from other assembly code, where we have full control of the stack

; void* __callobf_buildSpoofedCallStack(PSTACK_SPOOF_INFO)
__callobf_buildSpoofedCallStack:
    set_opt stored_regs             , rbp, rbx, rsi, r12, r13, r14, r15
    set_opt arg_count               , 1
    set_opt local_var_space         , 0x0
    set_opt max_callee_arg_count    , 1
    set_opt store_args              , false
    
    end_opts

; ======================================================================================================================


; PSTACK_SPOOF_INFO p_info
%define p_info rcx

%define listEntryCount      rsi
%define entry32             ebx
%define entry64             rbx

%define p_setFpRegFrameInfo r10
%define p_saveRbpFrameInfo  r11
%define p_jmpRbxFrameInfo   r12
%define p_addRspFrameInfo   r13
%define desiredRbpValue     r14
%define ciclicValue32       r15d

; ======================================================================================================================
    ; TODO: This hole thing looks ugly...
    checkNull p_info, error
    mov rbp, rsp
    mov rbx, rcx ; Temporary save rcx

;   Initialize spoof info if not already initialized
    mov rax, [fieldPtr(STACK_SPOOF_INFO, p_info, initialized)]
    test rax, rax
    jnz alreadyInitialized

    fastcall_win64 __callobf_initializeSpoofInfo, p_info
    checkNull rax, error

    mov rcx, rbx
alreadyInitialized:
;   Obtain a new value to get the indexes in every list
    fastcall_win64 __callobf_lfsrXorShift32, [fieldPtr(STACK_SPOOF_INFO, p_info, nextCiclicValue)]
    mov rcx, rbx

;   Save it
    mov DWORD [fieldPtr(STACK_SPOOF_INFO, p_info, nextCiclicValue)], eax

;   We need rax to be used in the mod macro (div inside)
    mov  ciclicValue32, eax

; For every list, pick a byte, put the value in range of each list entry count, and get the entry
    pickByte entry32, ciclicValue32, 0
    mod entry32, DWORD [fieldPtr(STACK_SPOOF_INFO, p_info, setFpRegCount)]
    shl entry64, 4 ; Multiply by 16 (sizeof(FRAME_INFO))
    lea p_setFpRegFrameInfo, [fieldPtr(STACK_SPOOF_INFO, p_info, setFpRegList) + entry64]

    pickByte entry32, ciclicValue32, 1
    mod entry32, DWORD [fieldPtr(STACK_SPOOF_INFO, p_info, jmpRbxCount)]
    shl entry64, 4 ; Multiply by 16 (sizeof(FRAME_INFO))
    lea p_jmpRbxFrameInfo, [fieldPtr(STACK_SPOOF_INFO, p_info, jmpRbxList) + entry64]

    pickByte entry32, ciclicValue32, 2
    mod entry32, DWORD [fieldPtr(STACK_SPOOF_INFO, p_info, addRspCount)]
    shl entry64, 4 ; Multiply by 16 (sizeof(FRAME_INFO))
    lea p_addRspFrameInfo, [fieldPtr(STACK_SPOOF_INFO, p_info, addRspList) + entry64]

    pickByte entry32, ciclicValue32, 3
    mod entry32, DWORD [fieldPtr(STACK_SPOOF_INFO, p_info, saveRbpCount)]

    ; Since size of SAVE_RBP_FRAME_INFO is 24 bytes, means rax * 24
    mov r11, entry64 ; tmp holder

    ; Multiply by 24 (sizeof(SAVE_RBP_FRAME_INFO))
    ; rax * 24 = 8 * ((rax * 2) + rax)
    shl entry64, 1      ; (rax * 2)
    add entry64, r11    ; + "rax"
    shl entry64, 3      ; rax * 8

    lea  p_saveRbpFrameInfo, [fieldPtr(STACK_SPOOF_INFO, p_info, saveRbpList) + entry64]

;   Calculate the rbp value that will be "saved" by saveRbpFrame
    mov desiredRbpValue, [fieldPtr(STACK_SPOOF_INFO, p_info, p_entryRetAddr)]
    sub desiredRbpValue, [fieldPtr(FRAME_INFO, p_setFpRegFrameInfo, frameSize)]

;   Set startting frame, used to set an arbitrary rsp value, based on rbp
    mov  rax, [fieldPtr(FRAME_INFO, p_setFpRegFrameInfo, p_entryAddr)]
    checkNull rax, error
    push rax

;   Set frame to load arbitrary rbp value
    sub  rsp, [fieldPtr(SAVE_RBP_FRAME_INFO, p_saveRbpFrameInfo, frameSize)]
    mov  rax, [fieldPtr(SAVE_RBP_FRAME_INFO, p_saveRbpFrameInfo, p_entryAddr)]
    checkNull rax, error
    push rax

;   Store desiredRbpValue at the address where it should have been saved ?¿ <This sounds horrible>
    mov rax,  [fieldPtr(SAVE_RBP_FRAME_INFO, p_saveRbpFrameInfo, rbpOffset)]
    add rax, 0x8
    mov [rax + rsp], desiredRbpValue

;   Set frame to redirect execution to the restore routine, it will need to be stored in rbx before execution begins, caller job
    sub  rsp, [fieldPtr(FRAME_INFO, p_jmpRbxFrameInfo, frameSize)]
    mov  rax, [fieldPtr(FRAME_INFO, p_jmpRbxFrameInfo, p_entryAddr)]
    checkNull rax, error
    push rax

;   Set frame to add some stack space so we can store args, shadow space, etc.
    sub  rsp, [fieldPtr(FRAME_INFO, p_addRspFrameInfo, frameSize)]
    mov  rax, [fieldPtr(FRAME_INFO, p_addRspFrameInfo, p_entryAddr)]
    checkNull rax, error
    push rax

;   Return value of spoofed stack start
    mov rax, rsp

success: 
    jmp end

error:
    xor rax, rax

end:
    mov rsp, rbp
    end_function