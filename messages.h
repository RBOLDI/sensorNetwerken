/*
 * messages.h
 *
 * Created: 11-9-2019 14:40:52
 *  Author: Steph
 */ 

#define		ROUTINGHEADER	0x02
#define		DATAHEADER		0x03

#define COLOR_RED			"\033[0;31m"
#define COLOR_BOLD_RED		"\033[1;31m"
#define COLOR_GREEN			"\033[0;32m"
#define COLOR_BOLD_GREEN	"\033[1;32m"
#define COLOR_YELLOW		"\033[0;33m"
#define COLOR_BOLD_YELLOW	"\033[1;33m"
#define COLOR_BLUE			"\033[0;34m"
#define COLOR_BOLD_BLUE		"\033[1;34m"
#define COLOR_MAGENTA		"\033[0;35m"
#define COLOR_BOLD_MAGENTA	"\033[1;35m"
#define	COLOR_CYAN			"\033[0;36m"
#define COLOR_BOLD_CYAN		"\033[1;36m"
#define COLOR_RESET			"\033[0m"

#define BASESTATION_ID 105

//Function protoypes.
uint8_t* pipe_selector(uint8_t ID);
uint8_t GetIdFromLookup(uint8_t* serial);
void printf_hex(uint8_t *str, uint8_t str_len);
void printf_bin(uint8_t *str, uint8_t str_len);
void printf_Routing(uint8_t *str, uint8_t str_len);
void printf_DeviceSerial(uint8_t *str, uint8_t str_len);
void printf_SetColor(char* cc);

// Global variable
uint8_t MYID;
