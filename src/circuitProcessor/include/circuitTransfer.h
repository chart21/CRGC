#ifndef CIRCUIT_TRANSFER_H__
#define CIRCUIT_TRANSFER_H__

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>

using namespace std;
#define NETWORK_BUFFER_SIZE 1024*1024

template<typename T>
class IO {
private:
	T& derived() {
		return *static_cast<T*>(this);
	}
public:
    uint64_t counter = 0;
	void send_data(const void * data, size_t nbyte) {
		counter +=nbyte;
		derived().send_data_internal(data, nbyte);
	}

	void recv_data(void * data, size_t nbyte) {
		derived().recv_data_internal(data, nbyte);
	}
};

class FileIO : public IO<FileIO> {
public:
    uint64_t bytes_sent = 0;
	FILE * stream = nullptr;
	char * buffer = nullptr;
	FileIO(const string filepath, bool read) {
		if (read)
			stream = fopen((filepath+"_compressed.dat").c_str(),"r");
		else
			stream = fopen((filepath+"_compressed.dat").c_str(),"w");

	}

	~FileIO(){
		fflush(stream);
		fclose(stream);
	}

	void flush() {
		fflush(stream);
	}

	void reset() {
		rewind(stream);
	}

	void send_data_internal(const void *data, size_t len) {
        //fwrite(data,sizeof(data[0]),len,stream);
		fwrite(data,1,len,stream);
		// bytes_sent += len;
		// int sent = 0;
		// while(sent < len) {
		// 	int res = fwrite(sent+(char*)data, 1, len-sent, stream);
		// 	if (res >= 0)
		// 		sent+=res;
		// 	else
		// 		fprintf(stderr,"error: file_send_data %d\n", res);
		// }
	}
	void recv_data_internal(void *data, size_t len) {
        fread(data, 1, len, stream);
		// int sent = 0;
		// while(sent < len) {
		// 	int res = fread(sent+(char*)data, 1, len-sent, stream);
		// 	if (res >= 0)
		// 		sent+=res;
		// 	else 
		// 		fprintf(stderr,"error: file_recv_data %d\n", res);
		// }
	}
};

class NetIO : public IO<NetIO> { public:
	bool is_server;
	int mysocket = -1;
	int consocket = -1;
	FILE * stream = nullptr;
	char * buffer = nullptr;
	bool has_sent = false;
	string addr;
	int port;
	NetIO(const char * address, int port, bool quiet = false) {
		port = port & 0xFFFF;
		is_server = (address == nullptr);
		if (address == nullptr) {
			struct sockaddr_in dest;
			struct sockaddr_in serv;
			socklen_t socksize = sizeof(struct sockaddr_in);
			memset(&serv, 0, sizeof(serv));
			serv.sin_family = AF_INET;
			serv.sin_addr.s_addr = htonl(INADDR_ANY); /* set our address to any interface */
			serv.sin_port = htons(port);           /* set the server port number */    
			mysocket = socket(AF_INET, SOCK_STREAM, 0);
			int reuse = 1;
			setsockopt(mysocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse));
			if(bind(mysocket, (struct sockaddr *)&serv, sizeof(struct sockaddr)) < 0) {
				perror("error: bind");
				exit(1);
			}
			if(listen(mysocket, 1) < 0) {
				perror("error: listen");
				exit(1);
			}
			consocket = accept(mysocket, (struct sockaddr *)&dest, &socksize);
			close(mysocket);
		}
		else {
			addr = string(address);
			
			struct sockaddr_in dest;
			memset(&dest, 0, sizeof(dest));
			dest.sin_family = AF_INET;
			dest.sin_addr.s_addr = inet_addr(address);
			dest.sin_port = htons(port);

			while(1) {
				consocket = socket(AF_INET, SOCK_STREAM, 0);

				if (connect(consocket, (struct sockaddr *)&dest, sizeof(struct sockaddr)) == 0) {
					break;
				}
				
				close(consocket);
				usleep(1000);
			}
		}
		set_nodelay();
		stream = fdopen(consocket, "wb+");
		buffer = new char[NETWORK_BUFFER_SIZE];
		memset(buffer, 0, NETWORK_BUFFER_SIZE);
		setvbuf(stream, buffer, _IOFBF, NETWORK_BUFFER_SIZE);
		if(!quiet)
			std::cout << "connected\n";
	}

	void sync() {
		int tmp = 0;
		if(is_server) {
			send_data_internal(&tmp, 1);
			recv_data_internal(&tmp, 1);
		} else {
			recv_data_internal(&tmp, 1);
			send_data_internal(&tmp, 1);
			flush();
		}
	}

	~NetIO(){
		flush();
		fclose(stream);
		delete[] buffer;
	}

	void set_nodelay() {
		const int one=1;
		setsockopt(consocket,IPPROTO_TCP,TCP_NODELAY,&one,sizeof(one));
	}

	void set_delay() {
		const int zero = 0;
		setsockopt(consocket,IPPROTO_TCP,TCP_NODELAY,&zero,sizeof(zero));
	}

	void flush() {
		fflush(stream);
	}

	void send_data_internal(const void * data, size_t len) {
		size_t sent = 0;
		while(sent < len) {
			size_t res = fwrite(sent + (char*)data, 1, len - sent, stream);
			if (res >= 0)
				sent+=res;
			else
				fprintf(stderr,"error: net_send_data %d\n", res);
		}
		has_sent = true;
	}

	void recv_data_internal(void  * data, size_t len) {
		if(has_sent)
			fflush(stream);
		has_sent = false;
		int sent = 0;
		while(sent < len) {
			int res = fread(sent + (char*)data, 1, len - sent, stream);
			if (res >= 0)
				sent += res;
			else 
				fprintf(stderr,"error: net_send_data %d\n", res);
		}
	}
};
#endif