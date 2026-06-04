#include <stdio.h>    	// standard input/output header
#include <stdlib.h>   	// Standard library header
#include <string.h>   	// String handling functions header
#include <unistd.h>   	// use read(), write(), close()
#include <arpa/inet.h>  // Network address conversion functions
#include <sys/socket.h> // Socket-related functions
#include <getopt.h>  	// Command-line option parsing header
#include <stdint.h>  	// Fixed-width integer types

#include "../edge/byte_op.h"  // include byte_op.h header

#define BUFLEN        1024   // define Buffer size as 1024
#define OPCODE_SUM    1      // define OPCODE_SUM as 1
#define OPCODE_REPLY  2 	 // define OPCODE_REPLY as 2

void protocol_execution(int sock);  // Declare protocol execution function
void error_handling(const char *message);  // Declare error handling function


//  Program usage print function
void usage(const char *pname)
{
  printf(">> Usage: %s [options]\n", pname);            // print usage
  printf("Options\n");                   	        	// print option list
  printf("  -a, --addr       Server's address\n");		// print server address option
  printf("  -p, --port       Server's port\n");    		// print server port option
  exit(0);												// exit program
}

int main(int argc, char *argv[])
{
	int sock;											// socket discripter
	struct sockaddr_in serv_addr;						// address information struct
  char msg[] = "Hello, World!\n";						// unused
	char message[30] = {0, };							// unused
	int c, port, tmp, str_len;							// c: store getopt_Long() return value, port : store port number, tmp: store string length, str_len : unused
  char *pname;											// store program name
  uint8_t *addr;										// store server address
  uint8_t eflag;										// store error flag for input validation

  pname = argv[0];                                      // program name
  addr = NULL;											// reset address
  port = -1;											// reset port (-1)
  eflag = 0;											// reset error flag	

  while (1)
  {
    int option_index = 0;								// current option index
    static struct option long_options[] = {      		// Define long options (--addr, --port) for getopt_long()
      {"addr", required_argument, 0, 'a'},
      {"port", required_argument, 0, 'p'},
      {0, 0, 0, 0}
    };

    const char *opt = "a:p:0";							// Set -a and -p to require arguments

    c = getopt_long(argc, argv, opt, long_options, &option_index);    // Parse command-line arguments and retrieve options

    if (c == -1)					// Exit loop if no more options remain
      break;

    switch (c)
    {
      case 'a':
        tmp = strlen(optarg);               // Calculate server address string length
        addr = (uint8_t *)malloc(tmp);		// Allocate memory for address storage
        memcpy(addr, optarg, tmp);			// Allocate memory to store the address
        break;

      case 'p':
        port = atoi(optarg);				// Store the input server address in memory
        break;

      default:
        usage(pname);						// Print usage and exit on invalid option
    }
  }

  if (!addr)								// Set error flag if server address is missing
  {
    printf("[*] Please specify the server's address to connect\n");
    eflag = 1;
  }

  if (port < 0)								// Set error flag if port number is invalid
  {
    printf("[*] Please specify the server's port to connect\n");
    eflag = 1;
  }

  if (eflag)								// Print usage and exit if input error occurs
  {
    usage(pname);
    exit(0);
  }

	sock = socket(PF_INET, SOCK_STREAM, 0);		// Create TCP socket
	if (sock == -1)
		error_handling("socket() error");		// Handle socket creation failure
	memset(&serv_addr, 0, sizeof(serv_addr));	// Initialize server address structure to zero
	serv_addr.sin_family = AF_INET;				// Set IPv4 address family
	serv_addr.sin_addr.s_addr = inet_addr((const char *)addr);		// Convert string IP address to network byte order and set it
	serv_addr.sin_port = htons(port);			// Convert port number to network byte order and set it

	if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)   // Try connecting to server, print connection info on success, handle error on failure
		error_handling("connect() error");							
  printf("[*] Connected to %s:%d\n", addr, port);
  
  protocol_execution(sock);  	// Perform protocol communication with server

	close(sock);
	return 0;					// Close socket and exit program normally
}

void protocol_execution(int sock)
{
  char msg[] = "Alice";
  char buf[BUFLEN];
  int tbs, sent, tbr, rcvd, offset;
  int len;

  // tbs: the number of bytes to send
  // tbr: the number of bytes to receive
  // offset: the offset of the message

  // 1. Alice -> Bob: length of the name (4 bytes) || name (length bytes)
  // Send the length information (4 bytes)
  len = strlen(msg);
  printf("[*] Length information to be sent: %d\n", len);

  len = htonl(len);
  tbs = 4;
  offset = 0;

  while (offset < tbs)
  {
    sent = write(sock, &len + offset, tbs - offset);
    if (sent > 0)
      offset += sent;
  }

  // Send the name (Alice)
  tbs = ntohl(len);
  offset = 0;

  printf("[*] Name to be sent: %s\n", msg);
  while (offset < tbs)
  {
    sent = write(sock, msg + offset, tbs - offset);
    if (sent > 0)
      offset += sent;
  }

  // 2. Bob -> Alice: length of the name (4 bytes) || name (length bytes)
  // Receive the length information (4 bytes)
  tbr = 4;
  offset = 0;

  while (offset < tbr)
  {
	  rcvd = read(sock, &len + offset, tbr - offset);
    if (rcvd > 0)
      offset += rcvd;
  }
  len = ntohl(len);
  printf("[*] Length received: %d\n", len);

  // Receive the name (Bob)
  tbr = len;
  offset = 0;

  while (offset < tbr)
  {
    rcvd = read(sock, buf + offset, tbr - offset);
    if (rcvd > 0)
      offset += rcvd;
  }

	printf("[*] Name received: %s \n", buf);

  // Implement following the instructions below
  // Let's assume there are two opcodes:
  //     1: summation request for the two arguments
  //     2: reply with the result
  // 3. Alice -> Bob: opcode (4 bytes) || arg1 (4 bytes) || arg2 (4 bytes)
  // The opcode should be 1

  char *p;
  int i, arg1, arg2;

  memset(buf, 0, BUFLEN);
  p = buf;
  arg1 = 2;
  arg2 = 5;

  VAR_TO_MEM_1BYTE_BIG_ENDIAN(OPCODE_SUM, p);
  VAR_TO_MEM_4BYTES_BIG_ENDIAN(arg1, p);
  VAR_TO_MEM_4BYTES_BIG_ENDIAN(arg2, p);
  tbs = p - buf;
  offset = 0;

  printf("[*] # of bytes to be sent: %d\n", tbs);
  printf("[*] The following bytes will be sent\n");
  for (i=0; i<tbs; i++)
    printf ("%02x ", buf[i]);
  printf("\n");

  while (offset < tbs)
  {
    sent = write(sock, buf + offset, tbs - offset);
    if (sent > 0)
      offset += sent;
  }

  // 4. Bob -> Alice: opcode (4 bytes) || result (4 bytes)
  // The opcode should be 2

  int opcode, result;

  tbr = 8; offset = 0;
  memset(buf, 0, BUFLEN);

  printf("[*] # of bytes to be received: %d\n", tbr);
  while (offset < tbr)
  {
    rcvd = read(sock, buf + offset, tbs - offset);
    if (rcvd > 0)
      offset += rcvd;
  }

  printf("[*] The following bytes is received\n");
  for (i=0; i<tbr; i++)
    printf("%02x ", buf[i]);
  printf("\n");

  p = buf;
  MEM_TO_VAR_4BYTES_BIG_ENDIAN(p, opcode);
  printf("[*] Opcode: %d\n", opcode);
  MEM_TO_VAR_4BYTES_BIG_ENDIAN(p, result);
  printf("[*] Result: %d\n", result);
}

void error_handling(const char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
