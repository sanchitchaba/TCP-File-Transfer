#include<iostream>
#include<sys/socket.h>
#include<netinet/in.h>	
#include<unistd.h>
#include<string.h>
#include<cstdlib>	
#include<fstream>

using namespace std;

void ls(int, char[]);
void download_c(int , char[]);
void upload_c(int, char[]);

int main(){

	int serv_fd,opt = 1, client_fd;
	char msg[] = "Message received!\n";

	struct sockaddr_in address;

	int addr_len = sizeof(address);
	char buffer[1024] = {0};

	serv_fd = socket(AF_INET, SOCK_STREAM, 0);

	if(serv_fd == 0){
		cout<<"Error - Socket Creation failed.";
		exit(EXIT_FAILURE);
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(6001);

	if(bind(serv_fd, (struct sockaddr *)&address, sizeof(address)) < 0){
		cout<<"Error - problem in binding.";
		exit(EXIT_FAILURE);
	}	

	if(listen(serv_fd, 3) < 0){
		cout<<"Error - problem in listening.";
		exit(EXIT_FAILURE);
	}

	if((client_fd = accept(serv_fd,(struct sockaddr *)&address, (socklen_t*)&addr_len)) < 0){
		cout<<"Error - problem in accepting client connection.";
		exit(EXIT_FAILURE);
	}

	cout<<"Connection established!\n";
	memset(buffer,'\0',sizeof(buffer));

	while(true){
		read(client_fd, buffer, 1024);
		cout<<"Client - "<<buffer<<endl;

		if(strcmp(buffer,"exit") == 0){
			cout<<"exiting connection!\n";
			break;
		} else if(strncmp(buffer,"$ls",3) == 0){
			cout<<"list called!"<<endl;
			ls(client_fd, &buffer[1]);
		} else if(strncmp(buffer,"$upload",7) == 0){
			cout<<"upload";
			cout<<"file -"<<&buffer[8]<<endl;

			int loc=7;
			for(int i=0;buffer[i]!=-1;i++) if(buffer[i] == '/') loc = i;

			cout<<"location : "<<loc<<endl;
			upload_c(client_fd, &buffer[++loc]);
		} else if(strncmp(buffer,"$download",9) == 0) download_c(client_fd, &buffer[10]);
		else write(client_fd, msg, strlen(msg));

		memset(buffer,'\0',strlen(buffer));
	}

	return 0;
}

void ls(int client_fd,char command[]){
	char buffer[1024] = {0};

	strcat(command, " > list.txt");
	system(command);

	ifstream fin;
	fin.open("list.txt");

	while(!fin.eof()){
		fin.getline(buffer, 1024);
		buffer[strlen(buffer)]='\n';

		cout<<buffer<<endl;
		write(client_fd, buffer, strlen(buffer));
		memset(buffer,'\0',strlen(buffer));
	}

	char terminate[] = "#";
	write(client_fd,terminate,strlen(terminate));

	fin.close();
}

void download_c(int client_fd, char file[]){
	char buffer[1024] = {0};

	int buf=0,buffer_counter=0;
	ifstream fin;

	fin.open(file,ios::binary);

	if(!fin){
		buffer[0]=-2;
		write(client_fd,buffer,sizeof(buffer));
		cout<<"File not found!"<<endl;
	}
	else{
		while((buf=fin.get())!= -1){
			if(buffer_counter == 1023){
				buffer[buffer_counter] = 'n';

				write(client_fd, buffer, sizeof(buffer));
				int k=fin.tellg();
				buffer[0] = buf;
				buffer_counter = 1;
			} else buffer[buffer_counter++] = (char)buf;
		}

		buffer[1023]='y';
		
		if(buffer_counter != 1024) buffer[buffer_counter++] = 'E';
		if(buffer_counter != 1024) buffer[buffer_counter++] = 'N';
		if(buffer_counter != 1024) buffer[buffer_counter++] = 'D';

		cout<<buffer[1023]<<endl;
		write(client_fd, buffer, sizeof(buffer));

		cout<<"transmission ended !"<<endl;
	}

	fin.close();
}

void upload_c(int client_sock, char file[]){
	cout<<"in upload command of client i.e. server downloading"<<endl;
	cout<<file<<" file"<<endl;

	int buffer_counter = 0,end_byte;
	char buffer[1024] = {0}, search[] = "END";

	ofstream fout;

	read(client_sock, buffer, sizeof(buffer));

	if(buffer[0] == -2) cout<<"No File exist!"<<endl;
	else{
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
				if(buffer[buffer_counter++] == 'E'){
					end_byte = buffer_counter - 1;
					for(i = 1;i<=2 && search[i] == buffer[buffer_counter++] ; i++);

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