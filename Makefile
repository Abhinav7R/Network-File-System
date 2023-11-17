naming_server: tries.c lru.c ss_info.c nm_functions.c NM.c
	gcc -o naming_server tries.c lru.c ss_info.c nm_functions.c NM.c 

clean:
	rm -f naming_server	


