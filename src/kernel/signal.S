.global signal_call_sigreturn
signal_call_sigreturn:
    mov x8, #15
    svc 0