//
//  globalhelper.h
//  
//
//  Created by Zhu Ji on 3/27/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef _globalhelper_h
#define _globalhelper_h

#define MAXDATASIZE 300 // max number of bytes we can get at once 
#define max(A,B)	( (A) > (B) ? (A):(B)) 
#define absolute(a)     ( (a) >= (0) ? (a):-(a))
#define MAXNEIGHNUM  400 // maximum number of neighhours we can hold;
#define ONETIMEALLOC 100 // every time when the buffer is full, allocate a new buffer with additional this long.
#define STRINGBUFFERLENGTH 50; // the length of string buffer;
#define MAXHOPNUM  100 // max number of hops
#define MAXMESSAGESIZE 500
struct NetworkTopoStruct *pGlobalTopo = NULL;


#define int16 short
#define bool int





#endif
