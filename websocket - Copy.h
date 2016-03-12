/* 
 * File:   websocket.h
 * Author: frans
 *
 * Created on 1 maart 2016, 17:29
 */

#ifndef WEBSOCKET_H
#define	WEBSOCKET_H



#define MaxFrameLength 248   
#define WebSocketKeyLength 24 

unsigned char GetClientKeyIdent(unsigned char);
void replyHandshake(void);

typedef struct {
    unsigned char Opcode : 4;
    unsigned char RSV : 3;
    unsigned char FIN : 1;
    unsigned char PayloadLen : 7;
    unsigned char Mask : 1;
    unsigned char data[MaxFrameLength+6]; //allow room for additional (FZ: ????)
} WebSocketFrame;
  




#endif	/* WEBSOCKET_H */

