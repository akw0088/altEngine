lcc -DVM -S -Wf-target=bytecode -Wf-g vm_main.c -o obj/vm_main.asm
lcc -DVM -S -Wf-target=bytecode -Wf-g vm_stdlib.c -o obj/vm_stdlib.asm
q3asm -o c:\altEngine2\media\vm\game.qvm obj/vm_main.asm obj/vm_stdlib.asm vm_syscalls.asm