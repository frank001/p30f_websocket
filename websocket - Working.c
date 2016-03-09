#include <p30f4013.h>
#include "websocket.h"

extern volatile unsigned char i2c_reg_map[];    //Debugging only


unsigned char clientkey[WebSocketKeyLength];
unsigned int keyidentcntr = 0;
unsigned char i;                            //debugging

unsigned char keyfound = 0;                 //TODO: flag register/struct
unsigned char firstpass = 0;


unsigned char WebSocketKeyIdent[] = "Sec-WebSocket-Key: ";
char WebSocketKey[] = "dGhlIHNhbXBsZSBub25jZQ==";

unsigned char WebSocketKeyRead[32];              //debugging

unsigned char GetClientKeyIdent(unsigned char i2c_rcv) {
    
    //TODO: clean up by better if-then branching
    
    //store the client key once the identification has been found
    if (keyfound==1 && firstpass==0 && keyidentcntr<WebSocketKeyLength) {
        //i2c_reg_map[0x10 + keyidentcntr] = i2c_rcv;                                       //log only put this into WebSocketKey
        WebSocketKey[keyidentcntr] = i2c_rcv;
        if (keyidentcntr==WebSocketKeyLength-1) {
            firstpass = 1;
            //we've got the key! now hash it and reply!
            for (i=0;i<WebSocketKeyLength;i++) i2c_reg_map[i+0x10] = WebSocketKey[i];   //debugging
        }
        keyidentcntr++;
    }
    
    //lets see if we find a client web-socket key identification string
    if (!keyfound && i2c_rcv==WebSocketKeyIdent[keyidentcntr]) {
        //WebSocketKeyRead[keyidentcntr] = i2c_rcv;                       //log for debugging, the increment is essential!!!
        keyidentcntr++;
    }
    //check if the key identification is still valid
    if (!keyfound && keyidentcntr==sizeof(WebSocketKeyIdent) && i2c_rcv!=WebSocketKeyIdent[keyidentcntr]) {
        //abort. this can't be the key identification
        keyidentcntr=0;
    }
    //check for complete key
    if (!keyfound && keyidentcntr==sizeof(WebSocketKeyIdent)-1) {
        //key identification found!!
        //for (keyidentcntr=0;keyidentcntr<sizeof(WebSocketKeyIdent);keyidentcntr++)         //log for debugging
        //    i2c_reg_map[keyidentcntr+0x10] = WebSocketKeyRead[keyidentcntr];               
        keyfound = 1;
        LATBbits.LATB2 = 1;
        //read the next 24 chars for the key itself!!!!
        keyidentcntr = 0;
    }
    
    
    return 0;
    
    /*//lets see if we find a client websocket key ident string
            rcv  = I2CRCV;
            if (rcv==WebSocketKeyIdent[keyidentcntr] && keyfound==0){
                clientkey[keyidentcntr++]=rcv;  //char found. store it
                
            }
            //check if the client ident is still valid
            if (clientkey>0 && keyidentcntr<sizeof(WebSocketKeyIdent) && rcv!=WebSocketKeyIdent[keyidentcntr]) {
                //nope, trash it.
                keyidentcntr=0;
            }
            //check if we found the complete ident string
            if (keyidentcntr==sizeof(WebSocketKeyIdent)-1) {
                //LATBbits.LATB2 = 1;
                keyfound = 1;
                
            } 
            if (keyfound && firstpass==0) { 
                firstpass=1;
                LATBbits.LATB2 = 1;          
                for (i=0;i<sizeof(WebSocketKeyIdent);i++) i2c_reg_map[i+0x10] = clientkey[i]; //debugging
                
            } //Found it! now read the key itself!!!!
 */    
    
}
