nyetwork: nyetwork.c
	gcc -o nyetwork.so -shared nyetwork.c -ldl -fPIC -I.
