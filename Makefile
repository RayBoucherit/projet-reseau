make :
	gcc -g -Wall -c -o tlv.o tlv.c;
	gcc -g -Wall -c -o package.o package.c;
	gcc -g -Wall -c -o hashtable.o hashtable.c
	gcc -g -Wall -c -o dazibao.o dazibao.c;
	gcc -g -Wall -c -o hash.o hash.c;
	gcc -g -Wall -o dazibao hashtable.o hash.o package.o tlv.o dazibao.o -lcrypto -lssl;

clean :
	rm dazibao *.o;
