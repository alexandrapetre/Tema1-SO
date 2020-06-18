CFLAGS = /D_CRT_SECURE_NO_DEPRECATE /W3 /nologo /EHsc /Za /MD

build: so-cpp.exe

so-cpp.exe: so-cpp.obj
	cl $(CFLAGS) /Fe$@ $**
	
so-cpp.obj: so-cpp.c
	cl $(CFLAGS) /Fe$@ /c $**

clean:
	del *.obj 
	del so-cpp.exe 
