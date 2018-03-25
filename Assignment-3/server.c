//server side for question 3 

#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <signal.h>
#include <sys/types.h>

int serv_socket = 0, new_connection = 0;

char xor(char a, char b) {
    if(a == b)
        return '0';
    else 
        return '1';
}

void compare(char *data, char *poly, int index) {
    int i, poly_len = strlen(poly);
    if(data[index] == '0')
        return;
    for(i = index; i<poly_len+index; i++) {
        data[i] = xor(data[i], poly[i - index]);
    }
}

//generating CRC checksum for the data using CRC-8 polynomial
void generate_crc(char *poly, char *data, char *checksum) {
    int i, data_len = strlen(data);
    char sample[50] = "";
    strcpy(sample, data);
    int poly_len = strlen(poly);
    strcpy(checksum, "");
    for(i = 0; i<poly_len-1; i++) {
        strcat(sample, "0");
    }
    for(i = 0; i<data_len; i++) {
        compare(sample, poly, i);
    }
    strcat(checksum, &sample[i]);
    strcat(data, &sample[i]);
}

//randomly insert corrupt bits in the data to be sent
void corrupt(char *data, float p) {
    int i, data_len = strlen(data);
    int corruptbits = rand()%data_len; //randomly deciding number of corrupt bits
    for(i = 1; i<corruptbits; i++) {
        float prob = (double)rand()/(double)RAND_MAX; //randomly assigning probability
        int index = rand()%data_len; //randomly deciding which index to change
        if(prob<p){
            if(data[index] == '0')
                data[index] = '1';
            else
                data[index] = '0';
        }
    }
}

//convert the port no. to integer
int chrtoint(char const *port) {
    int i, len = strlen(port);
    int port_no = 0, cur;
    for(i = 0; i<len; i++) {
    	cur = port[i] - '0';
        port_no += cur*pow(10, len-1-i);
    }
    return port_no;
}

//handling Ctrl+C
void inthand(int signum) {
	char c;
	printf("\nTake down the server?(y/n)\n");
	c = getchar();
	if(c == '\n')
		c = getchar();
	if(c == 'y' || c == 'Y') {
		printf("Taking down the server\n");
		close(serv_socket);
		close(new_connection);
	}
	return;
}

int main(int argc, char const *argv[]) {
	int port_no, read_value;
	struct sockaddr_in address;
	int addrlen = sizeof(address);
	char ack_message[50] = "00";
	char nack_message[50] = "01";
	char crc_polynomial[10] = "100000111"; //CRC-8 polynomial
    char accept_string[10] = "00000000";
    char checksum[50] = "";
    float ber = 0;
    printf("Enter BER rate: ");
    scanf("%f", &ber); //setting bit error rate
	signal(SIGINT, inthand); //interrupt handler
	port_no = chrtoint(argv[1]);
	printf("%d\n", port_no);

	while((serv_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Socket Creation Faliure\n"); //check whether there is some error in created socket or not
	} 

	address.sin_family = AF_INET; //IPv4 Internet Protocol
	address.sin_addr.s_addr = INADDR_ANY;//Accept any IP for bind
	address.sin_port = htons(port_no);//host to network short

	if(bind(serv_socket, (struct sockaddr *)&address, sizeof(address))<0) {
		printf("Bind Failed\n");//check whether socket is correctly associated with port or not
	}

	if(listen(serv_socket, 3)<0)//listen for connections
		printf("Error in Listening");

	while(1) {
	
	new_connection = accept(serv_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen);//new socket for incoming client connection
 	
 	if(new_connection < 0) {
 		break;// error in creating new_connection
 	}

 	if(fork() == 0) {//sucessfully create a child process


	while(1) {

		char buffer[1024] = "";
		char reply_packet[100] = "";
		read_value = recv(new_connection, buffer, 1024, 0); //message received form client
		if (read_value < 0) {
			printf("Error Reading from Socket\n");
			break;
		}
		if(strlen(buffer) == 0) {
			break;
		}
		printf("Message Received: %s\n", buffer);
		generate_crc(crc_polynomial, buffer, checksum);//insert error in received message
		if(strcmp(checksum, accept_string) == 0) {
			strcpy(reply_packet, ack_message);
			generate_crc(crc_polynomial, reply_packet, checksum);
			corrupt(reply_packet, ber);//insert error in ACK
			send(new_connection, reply_packet, strlen(reply_packet) , 0);//send ACK
			printf("ACK message sent\n");			
		} else {
			strcpy(reply_packet, nack_message);
			generate_crc(crc_polynomial, reply_packet, checksum);
			corrupt(reply_packet, ber);//insert error in NACK
			send(new_connection, reply_packet, strlen(reply_packet) , 0);//send NACK
			printf("NACK message sent\n");			

		}
	}
		break;

	} else {
		close(new_connection); //close the connection
	}
	
	}
	return 0;
}































