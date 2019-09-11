/*
 * messages.h
 *
 * Created: 11-9-2019 14:40:52
 *  Author: Steph
 */ 

// Defines used by message functions.
#define FULL_MESSAGE_SIZE 32
#define NUMBER_OF_PREFIX_BYTES 3
#define MAX_MESSAGE_SIZE FULL_MESSAGE_SIZE - NUMBER_OF_PREFIX_BYTES // Waarvan de laatste is '\0'

// Integers shared by message functions.
uint8_t initials[NUMBER_OF_PREFIX_BYTES];
uint8_t message[MAX_MESSAGE_SIZE];
uint8_t fullMessage[FULL_MESSAGE_SIZE];

//Function protoypes.
void sendMessage();
void sendPvtMessage(uint8_t targetID);
uint8_t* pipe_selector(uint8_t ID);