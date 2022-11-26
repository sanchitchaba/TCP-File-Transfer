#include<iostream>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<string.h>
#include<arpa/inet.h>
#include<fstream>

using namespace std;

void ls(int);
void download(int,char[]);
void upload(int, char[]);

int main(){
	int client_sock;
	struct sockaddr_in serv_addr;

	char msg[1024]= {0};
	char buffer[1024] = {0};

	client_sock = socket(AF_INET, SOCK_STREAM, 0);

	if(client_sock == 0){
		cout<<"Error - socket creation failed!";
		exit(EXIT_FAILURE);
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(6001);

	if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0){
		cout<<"Error - Invalid address!";
		return -1;
	}

	if(connect(client_sock,(struct sockaddr *)&serv_addr,sizeof(serv_addr))<0){
		cout<<"Error - Connection failed";
		return -1;
	}

	while(true){
		cout<<"Client: ";
		
		cin.getline(msg,sizeof(msg));
		write(client_sock, msg, strlen(msg));

		if(strncmp(msg,"$ls",3) == 0) ls(client_sock);
		else if(strncmp(msg,"$download",9) == 0){
			int loc = 9;
			for(int i=0;msg[i]!=-1;i++) if(msg[i]=='/') loc = i;

			download(client_sock, &msg[++loc]);
		} else if(strcmp(msg,"exit") == 0){
			cout<<"exiting connection!\n"<<endl;
			break;
		} else if(strncmp(msg,"$upload",7) == 0) upload(client_sock, &msg[8]);
		else{
			read(client_sock, buffer, 1024);
			buffer[strlen(buffer)] = '\0';
			cout<<"\nServer: "<<buffer;
		}

		memset(msg,'\0',strlen(msg));
	}

	return 0;
}

void ls(int client_sock){
	char buffer[1024] = {0};
	bool cont = true;

	cout<<"--------File------\n";

	while(cont){
		memset(buffer,'\0',strlen(buffer));
		read(client_sock, buffer, 1024);
		
		for(int i=0;i<strlen(buffer);++i){
			if(buffer[i]=='#'){
				cont = false;
				break;
			} else cout<<buffer[i];
		}
	}
}

void download(int client_sock, char file[]){
	cout<<"in download"<<endl;
 
	int buffer_counter=0,end_byte;
	char buffer[1024]={0},search[]="END";

	ofstream fout;

	read(client_sock, buffer, sizeof(buffer));

	if(buffer[0] == -2) cout<<"No File exist!"<<endl;
	else{
		cout<<file<<" file name"<<endl;
		fout.open(file);

		while(buffer[1023] == 'n'){
			fout.put((char)buffer[buffer_counter++]);

			if(buffer_counter == 1023){
				read(client_sock, buffer, sizeof(buffer));
				buffer_counter = 0;
			}
		}

		if(buffer[1023] == 'E') end_byte = 1022;
		else if(buffer[1023] == 'N') end_byte = 1021;
		else if(buffer[1023] == 'D') end_byte = 1020;
		else{
			bool cont = true;
			int i;

			while(cont){
				if(buffer[buffer_counter++]=='E'){
					end_byte = buffer_counter - 1;

					for(i=1;i<=2 && search[i]==buffer[buffer_counter++];i++);

					if(i == 3) cont = false;
				}
			}

			buffer_counter = 0;
		}

		if(end_byte == -1) end_byte++;
		while(buffer_counter != end_byte) fout.put((char)buffer[buffer_counter++]);

		cout<<(int)buffer[buffer_counter]<<" this"<<endl;
		cout<<"file received !"<<endl;
		fout.close();
	}
}

void upload(int client_fd,char file[]){
	char buffer[1024] = {0};

	int buf = 0, buffer_counter = 0;
	ifstream fin;

	fin.open(file,ios::binary);

	if(!fin){
		buffer[0] = -2;
		write(client_fd, buffer, sizeof(buffer));
		cout<<"File not found!"<<endl;
	} else {
		while((buf = fin.get()) != -1){
			if(buffer_counter == 1023){
				buffer[buffer_counter] = 'n';

				write(client_fd, buffer, sizeof(buffer));
				cout<<fin.tellg()<<endl;				

				buffer[0] = buf;
				buffer_counter = 1;
			} else buffer[buffer_counter++] = (char)buf;
		}

		buffer[1023] = 'y';
		
		if(buffer_counter != 1024) buffer[buffer_counter++] = 'E';
		if(buffer_counter != 1024) buffer[buffer_counter++] = 'N';
		if(buffer_counter != 1024) buffer[buffer_counter++] = 'D';

		cout<<buffer[1023]<<endl;

		write(client_fd, buffer, sizeof(buffer));
		cout<<"transmission ended !"<<endl;
	}

	fin.close();
}