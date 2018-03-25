//client side for question 3

#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

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
    int corruptbits = rand()%(data_len); //randomly deciding number of corrupt bits
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


int main(int argc, char const *argv[] ) {
	int client_socket, port_no, read_value;
	struct sockaddr_in server_address;
	int addrlen = sizeof(server_address);
    char message[50] = "110100111110001101"; 
	char crc_polynomial[10] = "100000111"; //CRC-8 polynomial
    char accept_string[10] = "00000000";
    char checksum[50] = "";
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    port_no = chrtoint(argv[2]);
    float ber = 0;
    printf("Enter BER rate: ");
    scanf("%f", &ber); //setting bit error rate

	while((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Socket Creation Faliure\n"); //check whether there is some error in created socket or not
	} 

    memset(&server_address, '0', sizeof(server_address));//fill memory with constant byte

	server_address.sin_family = AF_INET; //IPv4 Internet Protocol
	server_address.sin_port = htons(port_no);
    setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO,(const char *)&tv,sizeof(struct timeval));//set options on sockets

    if(inet_pton(AF_INET, argv[1], &server_address.sin_addr)<=0) //convert address to binary
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)//initiate connection on a socket
    {
        printf("\nConnection Failed \n");
    }

    while(1) {
        
        char data_packet[100] = "";
        char buffer[1024] = "";
        
        strcpy(data_packet, message);
        generate_crc(crc_polynomial, data_packet, checksum);
        corrupt(data_packet, ber);//insert error 
        
        send(client_socket, data_packet, strlen(data_packet), 0);//send msg to server
        printf("Hello message sent: %s\n", data_packet);
        
        read_value = read(client_socket , buffer, 1024);//read msg received from server
        printf("Message Received: %s\n", buffer);
        
        if(read_value < 0) {
            continue;
        }

        generate_crc(crc_polynomial, buffer, checksum);
        if(strcmp(checksum, accept_string) == 0) {
            if(buffer[0] == '0' && buffer[1] == '0') {
                printf("ACK Received\n");
                break;
            } else if (buffer[0] == '0' && buffer[1] == '1') {
                printf("NACK Received\nRetransmitting...\n");
            }
        } else {
                printf("Corrupt Packet Received\nSending Data Again\n");
        }

    }
    close(client_socket);//close connection
    return 0;

}































