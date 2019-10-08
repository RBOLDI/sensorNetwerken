/*
 * messages.h
 *
 * Created: 11-9-2019 14:40:52
 *  Author: Steph
 */ 

#define		BROADCAST	0x01
#define		RHDR		0x02
#define		DHDR		0x03
#define		BCREPLY		0x04

//Function protoypes.
uint8_t* pipe_selector(uint8_t ID);
uint8_t GetIdFromLookup(uint8_t* serial);
void printf_hex(uint8_t *str, uint8_t str_len);
void printf_bin(uint8_t *str, uint8_t str_len);
void printf_Routing(uint8_t *str, uint8_t str_len);
void printf_DeviceSerial(uint8_t *str, uint8_t str_len);