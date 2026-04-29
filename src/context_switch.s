.intel_syntax noprefix

.global context_switch
.type context_switch, @function

# void context_switch(uint32_t* old_esp_ptr, uint32_t new_esp);
#   [esp+4]  = old_esp_ptr  (where to save current process's ESP)
#   [esp+8]  = new_esp      (new process's saved ESP to load)

context_switch:
    pushad                    # save EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI
    mov  eax, [esp + 36]      # eax = old_esp_ptr  (4 ret + 32 pushad = 36)
    mov  [eax], esp           # *old_esp_ptr = current esp
    mov  esp, [esp + 40]      # esp = new_esp  (loaded from old stack frame)
    popad                     # restore registers from new stack
    ret                       # return to wherever EIP was saved on new stack

.att_syntax prefix
