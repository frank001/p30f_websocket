#include <p30f4013.h>
#include "websocket.h"
#include "sha1.h"
#include "base64.h"


extern volatile unsigned char i2c_reg_map[];
extern volatile unsigned int i2c_reg_addr;

unsigned char WebSocketKeyIdent[] = "Sec-WebSocket-Key: ";
char WebSocketKey[] = "dGhlIHNhbXBsZSBub25jZQ==";
BYTE WebSocketGuid [] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
BYTE ServerReply [] =
"HTTP/1.1 101 Switching Protocols\r\n"
"Upgrade: websocket\r\n"
"Connection: Upgrade\r\n"
"Sec-WebSocket-Accept: ";
unsigned int keycntr = 0;

unsigned char wsData[255];
unsigned char wsByteCount;
unsigned int payloadlen = sizeof(wsData);

unsigned char cntrCRLF = 0;

WebSocketFrame wsFrame;




void ResetFlags(void) { flags.value = 0; }

void AnswerClient(unsigned char *msg) {
    unsigned char mask[4];
    char decoded[255];
    
    unsigned char i;
    
    wsByteCount = 0;
    wsFrame.value = msg[wsByteCount++];      

    i2c_reg_addr=0;                 //we're gonna write, so set the pointer to zero
    
    switch (msg[0] & 0x0f) {    //opcode
        case 0x00:      //continuation frame
            break;
        case 0x01:      //text frame
            if (msg[1] & 0x80) {             
                payloadlen = msg[1] & 0x7f;  //TODO: check for payload length, limit this to minimize memory usage
                for (i=0;i<4;i++)               //read in the mask
                    mask[i] = msg[2+i];
                for (i=0;i<payloadlen;i++)      //decode the message
                    decoded[i] = msg[i+6]^mask[i%4];
                    
                //at this point we have the complete message -> invoke command handler
                //for (i=0;i<payloadlen;i++)      //write it out for now, debugging
                //    i2c_reg_map[i2c_reg_addr++] = decoded[i];
                
                
                //echo the message back for now
                //wsFrame.FIN = 1;
                //wsFrame.OPCODE = 1;
                //wsFrame.RSV = 0;
                i2c_reg_map[i2c_reg_addr++] = 0x81;     //FIN bit high and opcode=1
                i2c_reg_map[i2c_reg_addr++] = payloadlen;
                //i2c_reg_map[i2c_reg_addr++] = 0x1a;
                
                for (i=0;i<payloadlen;i++) {
                    i2c_reg_map[i2c_reg_addr++] = decoded[i];
                }
                
                
            } else {
                //TODO: check for mask. frames from clients should be masked otherwise disconnect.
            }
            i2c_reg_map[i2c_reg_addr] = 0;
            wsByteCount=0;
            break;
        case 0x02:      //binary frame
            break;
        case 0x08:      //connection close
            flags.KEYFOUND = 0;
            flags.SOCKETCONNECT = 0;
            i2c_reg_map[i2c_reg_addr++] = 0xff;     //specific for rpi2b0 tcpip_i2c -> disconnect socket
            break;
        case 0x09:      //ping
            break;
        case 0x0a:      //pong
                _LATB2 = 1;
                _LATB2 = 0;
            break;
        default:        //reserved
            break;

    }
    //i2c_reg_map[i2c_reg_addr] = 0;      //terminate, specific for rpi2b0 tcpipd_i2c
    wsByteCount=0;                      //set the web socket read pointer to zero
    payloadlen = sizeof(wsData);
    
}

void ReadClient(unsigned char i2c_rcv) {
    if (!flags.SOCKETCONNECT) {
        GetClientKeyIdent(i2c_rcv);
    } else {
        //read until null char received      //this is wrong! check payload length
        
        if (i2c_rcv!=0x00) {                 //here's the web socket frame! TODO: watch for payload length and figure out when the client has finished writing
        //if (wsByteCount==2) payloadlen = i2c_rcv & 0x7f;
        //if (wsByteCount<(payloadlen)) {
            wsData[wsByteCount++]=i2c_rcv;
        } else {        //we've got all of it. now analyze the frame
            _LATB2 = 1;
            AnswerClient(wsData);
        }
    }
}

void GetClientKeyIdent(unsigned char i2c_rcv) {
    if (!flags.KEYFOUND) {
        _LATB2 = 0;
        //lets see if we find a client web-socket key identification string
        if (i2c_rcv==WebSocketKeyIdent[keycntr]) {
            keycntr++;
        }
        //check if the key identification is still valid
        if (keycntr==sizeof(WebSocketKeyIdent) && i2c_rcv!=WebSocketKeyIdent[keycntr]) {
            keycntr=0;
        }
        //check for complete key
        if (keycntr==sizeof(WebSocketKeyIdent)-1) {
            //key identification found!!
            flags.KEYFOUND = 1;
            //read the next 24 chars for the key itself!!!!
            keycntr = 0;
        } 
    } else {
        //store the client key once the identification has been found
        if (keycntr<WebSocketKeyLength) {
            WebSocketKey[keycntr] = i2c_rcv;
            if (keycntr==WebSocketKeyLength-1) {
                //we've got the key! now hash it and reply!
                //but no too fast! wait for the client to finish his write...
                //watch for CRLF CRLF, we have not yet switched protocols so HTTP still applies
            }
            keycntr++;
        }
        //watch for CRLF CRLF
        if (i2c_rcv==0x0d || i2c_rcv ==0x0a) { 
            cntrCRLF++;
            if (cntrCRLF==3) Handshake();  
        } else if (cntrCRLF>0) cntrCRLF=0;
    }
    
}


void Handshake(void) {
    static HASH_SUM ReplyHash;
    char Sha1Result[20];
    char ResultBase64[40];
    unsigned char i;
    
    
    
    //hash it
    SHA1Initialize(&ReplyHash);
    SHA1AddData(&ReplyHash,(BYTE*)WebSocketKey, WebSocketKeyLength);
    SHA1AddData(&ReplyHash,(BYTE*)WebSocketGuid,(WORD)sizeof WebSocketGuid - 1);
    SHA1Calculate(&ReplyHash,(BYTE*)Sha1Result);
    
    //Base 64 encoding
    Base64encode(ResultBase64, Sha1Result, 20);
    
    //write out the reply
    i2c_reg_addr=0;                             //we're gonna write, so set the pointer to zero
    for (i=0;i<sizeof(ServerReply);i++) 
        i2c_reg_map[i2c_reg_addr++] = ServerReply[i];
    
    //write out the hash
    i2c_reg_addr--;
    for (i=0;i<28;i++)                                  //28????
        i2c_reg_map[i2c_reg_addr++] = ResultBase64[i];
    
    //i2c_reg_addr-=12;
    //i2c_reg_addr=sizeof(ServerReply)+sizeof(ResultBase64);
    //we need to comply to HTTP protocol so twice CRLF it is.
    i2c_reg_map[i2c_reg_addr++] = 0x0d;
    i2c_reg_map[i2c_reg_addr++] = 0x0a;
    i2c_reg_map[i2c_reg_addr++] = 0x0d;
    i2c_reg_map[i2c_reg_addr++] = 0x0a;
    i2c_reg_map[i2c_reg_addr] = 0x00;
    //for (i=0;i<32;i++) i2c_reg_map[i2c_reg_addr++] = 0xff;
    flags.SOCKETCONNECT = 1;
    
    
    
}
