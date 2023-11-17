naming_server: tries.c NM.c lru.c ss_info.c
	gcc -o naming_server tries.c NM.c lru.c ss_info.c

clean:
	rm -f naming_server	


