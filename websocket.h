/* 
 * File:   websocket.h
 * Author: frans
 *
 * Created on 3 maart 2016, 17:38
 */

#ifndef WEBSOCKET_H
#define	WEBSOCKET_H

#define WebSocketKeyLength 24

typedef union {                 //this can't be right?, TODO: investigate
    unsigned char value; 
    struct {
        unsigned char KEYFOUND : 1,
            SOCKETCONNECT : 1,
            ISCONNECTED : 1,
            SPARE1 : 1,
            SPARE2 : 1,
            SPARE3 : 1,
            SPARE4 : 1,
            SPARE5 : 1;
    };
} Flags;

static Flags flags;

typedef union {
    unsigned int value;
    unsigned char FIN : 1;
    unsigned char RSV : 3;
    unsigned char OPCODE : 4;
} WebSocketFrame;

void AnswerClient(unsigned char *);
void ResetFlags(void);
void ReadClient(unsigned char);
void GetClientKeyIdent(unsigned char);
void Handshake(void);

#endif	/* WEBSOCKET_H */

