	EXTERN ExFreePoolWithTag:PROC

.CODE

	_driver_unload PROC
		jmp ExFreePoolWithTag
	_driver_unload ENDP

END