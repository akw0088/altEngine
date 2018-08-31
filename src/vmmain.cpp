#include "include.h"
#include "vm.h"


// Callback from the VM: system function call
intptr_t systemCalls(vm_t* vm, intptr_t* args)
{
	int id = -1 - args[0];

	switch (id)
	{
	case -1: /* PRINTF */
		printf("%s", (const char*)VMA(1, vm));
		return 0;
	case -2: /* ERROR */
		fprintf(stderr, "%s", (const char*)VMA(1, vm));
		return 0;

	case -3: /* MEMSET */
		memset(VMA(1, vm), args[2], args[3]);
		return 0;

	case -4: /* MEMCPY */
		memcpy(VMA(1, vm), VMA(2, vm), args[3]);
		return 0;

	default:
		fprintf(stderr, "Bad system call: %ld", (long int)args[0]);
	}
	return 0;
}


int vm_main(char *qvmfile, int caller)
{
	vm_t vm;
	int retVal = -1;
	int size = 0;

	char *image = get_file(qvmfile, &size);
	if (!image)
	{
		return -1;
	}

	if (VM_Create(&vm, qvmfile, (uint8_t *)image, systemCalls) == 0)
	{
		retVal = VM_Call(&vm, caller);
	}

	VM_Free(&vm);
	delete [] image;

	return retVal;
}


