/*
 * messages.h
 *
 * Created: 11-9-2019 14:40:52
 *  Author: Steph
 */ 

// Defines used by message functions.
#define		FULL_MESSAGE_SIZE 32
#define		NUMBER_OF_PREFIX_BYTES 3
#define		MAX_MESSAGE_SIZE FULL_MESSAGE_SIZE - NUMBER_OF_PREFIX_BYTES // Waarvan de laatste is '\0'

#define		BROADCAST	0x01
#define		RHDR		0x02
#define		DHDR		0x03
#define		BCREPLY		0x04

// Integers shared by message functions.
uint8_t message[MAX_MESSAGE_SIZE];
uint8_t fullMessage[FULL_MESSAGE_SIZE];

//Function protoypes.
uint8_t* pipe_selector(uint8_t ID);
uint8_t GetIdFromLookup(uint8_t* serial);
void printf_hex(uint8_t *str, uint8_t str_len);
void printf_bin(uint8_t *str, uint8_t str_len);
void printf_Routing(uint8_t *str, uint8_t str_len);
void printf_DeviceSerial(uint8_t *str, uint8_t str_len);