/*
 * IncFile1.h
 *
 * Created: 20-9-2019 14:48:33
 *  Author: Steph
 */ 

#define DEBUGMODE

// File for enabaling debug messages throughout the project.
#ifdef DEBUGMODE
# define DB_MSG(x) printf x
#else
# define DB_MSG(x) do {} while (0)
#endif