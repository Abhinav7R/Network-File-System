all: naming_server storage_server

naming_server: tries.c lru.c ss_info.c nm_functions.c NM.c
	gcc -o naming_server tries.c lru.c ss_info.c nm_functions.c NM.c 

storage_server: ss_client_orders.c ss_nm_orders.c storage_server.c
	gcc -o storage_server ss_client_orders.c ss_nm_orders.c storage_server.c

client: client.c
	gcc -o client client.c	

clean:
	rm -f naming_server	storage_server


