#include <p30f4013.h>
#include "websocket.h"
#include "sha1.h"
#include "base64.h"


extern volatile unsigned char i2c_reg_map[];    //Debugging only
extern volatile unsigned int i2c_reg_addr;

unsigned char clientkey[WebSocketKeyLength];
unsigned int keyidentcntr = 0;
unsigned char i;                            //debugging

unsigned char keyfound = 0;                 //TODO: flag register/struct
unsigned char firstpass = 0;
unsigned char isConnected = 0;

unsigned char WebSocketKeyIdent[] = "Sec-WebSocket-Key: ";
char WebSocketKey[] = "dGhlIHNhbXBsZSBub25jZQ==";
BYTE WebSocketGuid [] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
BYTE ServerReply [] =
"HTTP/1.1 101 Switching Protocols\r\n"
"Upgrade: websocket\r\n"
"Connection: Upgrade\r\n"
"Sec-WebSocket-Accept: ";
WebSocketFrame wsFrame;
unsigned int wsByteCount = 0;

unsigned char socketConnected = 0;

unsigned char WebSocketKeyRead[32];              //debugging
unsigned int i2c_reg_debug;



unsigned char GetClientKeyIdent(unsigned char i2c_rcv) {
    
    //TODO: clean up by better if-then branching
    
    if (!keyfound) {
        //lets see if we find a client web-socket key identification string
        if (i2c_rcv==WebSocketKeyIdent[keyidentcntr]) {
            //WebSocketKeyRead[keyidentcntr] = i2c_rcv;                       //log for debugging, the increment is essential!!!
            keyidentcntr++;
        }
        //check if the key identification is still valid
        if (keyidentcntr==sizeof(WebSocketKeyIdent) && i2c_rcv!=WebSocketKeyIdent[keyidentcntr]) {
            //abort. this can't be the key identification
            keyidentcntr=0;
        }
        //check for complete key
        if (keyidentcntr==sizeof(WebSocketKeyIdent)-1) {
            //key identification found!!
            //for (keyidentcntr=0;keyidentcntr<sizeof(WebSocketKeyIdent);keyidentcntr++)         //log for debugging
            //    i2c_reg_map[keyidentcntr+0x10] = WebSocketKeyRead[keyidentcntr];               
            keyfound = 1;
            LATBbits.LATB2 = 1;
            //read the next 24 chars for the key itself!!!!
            keyidentcntr = 0;
        } 
    } else {
        //store the client key once the identification has been found
        if (firstpass==0 && keyidentcntr<WebSocketKeyLength) {
            //i2c_reg_map[0x10 + keyidentcntr] = i2c_rcv;                                       //log only put this into WebSocketKey
            WebSocketKey[keyidentcntr] = i2c_rcv;
            if (keyidentcntr==WebSocketKeyLength-1) {
                firstpass = 1;
                //we've got the key! now hash it and reply!
                //for (i=0;i<WebSocketKeyLength;i++) i2c_reg_map[i+0x10] = WebSocketKey[i];   //debugging
                replyHandshake();
            }
            keyidentcntr++;
        }
    }
    
    return 0;
}

void replyHandshake(void) {
    static HASH_SUM ReplyHash;
    char Sha1Result[20];
    char ResultBase64[40];
    
    //hash it
    SHA1Initialize(&ReplyHash);
    SHA1AddData(&ReplyHash,(BYTE*)WebSocketKey, WebSocketKeyLength);
    SHA1AddData(&ReplyHash,(BYTE*)WebSocketGuid,(WORD)sizeof WebSocketGuid - 1);
    SHA1Calculate(&ReplyHash,(BYTE*)Sha1Result);
    
    //Base 64 encoding
    Base64encode(ResultBase64, Sha1Result, 20);
    
    //write out the reply
    for (i=0;i<sizeof(ServerReply);i++) i2c_reg_map[i+0x10] = ServerReply[i];
    
    //write out the hash
    for (i=0;i<sizeof(ResultBase64);i++) i2c_reg_map[i+sizeof(ServerReply)+0x0f] = ResultBase64[i];
    i2c_reg_map[sizeof(ResultBase64)+sizeof(ServerReply)+0x0f] = 0;
    socketConnected = 1;
}
