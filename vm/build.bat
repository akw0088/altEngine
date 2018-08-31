lcc -DQ3_VM -S -Wf-target=bytecode -Wf-g g_main.c -o obj/g_main.asm
lcc -DQ3_VM -S -Wf-target=bytecode -Wf-g bg_lib.c -o obj/bg_lib.asm
q3asm -o c:\altEngine2\media\vm\game.qvm obj/g_main.asm obj/bg_lib.asm g_syscalls.asm