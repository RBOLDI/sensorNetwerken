/*
 * IncFile1.h
 *
 * Created: 20-9-2019 14:48:33
 *  Author: Steph
 */ 


// File for enabaling debug messages throughout the project.

#define DEBUGMODE

#ifdef DEBUGMODE
#define DB_MSG(str) do { printf("%s", str); } while( false )
#else
#define DB_MSG(str) do { } while ( false )
#endif