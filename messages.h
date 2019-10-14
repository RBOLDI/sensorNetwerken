/*
 * messages.h
 *
 * Created: 11-9-2019 14:40:52
 *  Author: Steph
 */ 

#define		ROUTINGHEADER	0x02
#define		DATAHEADER		0x03

//Function protoypes.
uint8_t* pipe_selector(uint8_t ID);
uint8_t GetIdFromLookup(uint8_t* serial);
void printf_hex(uint8_t *str, uint8_t str_len);
void printf_bin(uint8_t *str, uint8_t str_len);
void printf_Routing(uint8_t *str, uint8_t str_len);
void printf_DeviceSerial(uint8_t *str, uint8_t str_len);