/*************************************************
                                           
 ZEM 200                                          
                                                    
 serial.c 
                                                      
 Copyright (C) 2003-2004, ZKSoftware Inc.      		
                                                      
*************************************************/
//#include <string.h>
//#include <stdlib.h>
#include <unistd.h>
//#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <time.h>
#include "serial.h"
//#include "utils.h"
//#include "options.h"
#include "yaffsfs.h"
#include "arca.h"

#if 0
static int fdRS232 = -1;
static int fdRS0232 = -1;
static int fdRS1232 = -1;

static int MapBaudrate[V10_NUM_BAUDRATES]={
    B0, B50, B75, B110, B134, B150, B200, B300, B600, B1200, B1800, 
    B2400, B4800, B9600, B19200, B38400, B57600, B115200};


/** This array is used to map the size of the data byte to the value of the
 * target system. The library assume, that we allways have 5-8 bit per byte.
 */
static int MapDatasize[V10_NUM_DATASIZES]={CS5, CS6, CS7, CS8};

//extern int serial_getc(void);
//extern int serial_tstc(void);

int ConvertBaudrate(int baudrate)
{   
    if (baudrate <= 1200)   return V10_B1200;
    if (baudrate >= 115200) return V10_B115200;
    if (baudrate == 1800)   return V10_B1800;    
    if (baudrate == 2400)   return V10_B2400;
    if (baudrate == 4800)   return V10_B4800;
    if (baudrate == 9600)   return V10_B9600;
    if (baudrate == 19200)  return V10_B19200;
    if (baudrate == 38400)  return V10_B38400;
    if (baudrate == 57600)  return V10_B57600;
    
    return V10_B9600;
}

int RS232_SetParameters(int port, int Baudrate, int Datasize, int Parity, int FlowControl)
{
    struct termios options;

    if ((Baudrate < 0) || (Baudrate>=V10_NUM_BAUDRATES)) return 0;    
    if ((Datasize<0) || (Datasize>=V10_NUM_DATASIZES))	return 0;

    //Get the current options for the port...
    tcgetattr(port, &options);

    /* General Setup: the library is design for `raw mode'. Only 1 stop can be
     * configured. There's no special `break handling'. The input stream
     * shouldn't be modified. Therefore parity error aren't marked and the
     * input mapping is disabled.
     */
    cfmakeraw(&options);
    
    /* handle the handshaking according to the open flags */
    if (FlowControl)
	    options.c_cflag |= CRTSCTS;
    else
	    options.c_cflag &= ~CRTSCTS;

    options.c_cflag &= ~HUPCL;
    
    options.c_iflag &= ~(IXON|IXOFF|IXANY);

    /* Decide wether to block while waiting for character or return immediatly.
     */
    options.c_cc[VMIN]  = 0;
    options.c_cc[VTIME] = 50;
    
    /* Mask the character size bits and set data bits according to the
     * parameter.
     */
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= MapDatasize[Datasize]; 

    /* Set the handling of the parity bit.
     */
    switch (Parity){
	case V10_NONE:			     /* disable parity bit */
	    options.c_cflag &= ~PARENB;
	    options.c_iflag &= ~INPCK;
	    break;
	case V10_EVEN:			     /* even parity */
	    options.c_cflag |= PARENB;
	    options.c_cflag &= ~PARODD;
	    options.c_iflag &= ~IGNPAR;
	    options.c_iflag |= INPCK;	     /* removed "|ISTRIP" */
	    break;
	case V10_ODD:			     /* odd parity */
	    options.c_cflag |= PARENB;
	    options.c_cflag |= PARODD;
	    options.c_iflag &= ~IGNPAR;
	    options.c_iflag |= INPCK;	     /* removed "|ISTRIP" */
	    break;
	case V10_IGNORE:		     /* use parity but dont test */
	    options.c_cflag |= PARENB;
	    options.c_iflag |= IGNPAR;
	    break;
	default:
	    return 0;
    }

    /* We have to enable the receiver and set the port to local mode.
     */
    options.c_cflag |= (CLOCAL|CREAD);


    /* Set the baud rates according to the parameter.*/
    cfsetispeed(&options, MapBaudrate[Baudrate]);
    cfsetospeed(&options, MapBaudrate[Baudrate]);

    /* At last we must set the new options for the port.
     */
    tcsetattr(port, TCSANOW, &options);

    return 1;
}

int RS232_SetStopbits (int port, int Stops)
{
    struct termios options;

    /* Get the current options for the port... */
    if (Stops == 1){
	tcgetattr(port, &options);
	options.c_cflag &= ~CSTOPB;
	tcsetattr(port, TCSANOW, &options);
    }
    else if (Stops == 2){
	tcgetattr(port, &options);
	options.c_cflag |= CSTOPB;
	tcsetattr(port, TCSANOW, &options);
    }
    else
	return 0;

    return 1;
}

int RS232_SetTimeouts (int port, int TenthOfSeconds)
{
    struct termios options;

    if ( TenthOfSeconds <= 0 ) return 0;

    tcgetattr(port, &options);
    options.c_cc[VMIN] = 0;		     /* we want `interchar timeouts' */
    options.c_cc[VTIME] = TenthOfSeconds;
    tcsetattr(port, TCSANOW, &options);
    
    return 1;
}

//S1 485
//S2 console
//S3 external rs232 communication
//S0 MIFARE TTL 

// Initialise serial port at the request baudrate. 
static int arca_serial_init(int BaudRate, int DataSize, int Parity, int FlowControl)
{
   
#ifndef UCOS
    if (fdRS232 > 0) close(fdRS232);
#ifdef ZEM300
    if(gOptions.RS485On)
    	fdRS232 = open("/dev/ttyS1", O_RDWR | O_NOCTTY);
    else
    	fdRS232 = open("/dev/ttyUART1", O_RDWR | O_NOCTTY);
#else
    fdRS232 = open("/dev/ttyUART1", O_RDWR | O_NOCTTY);
#endif
    if ( fdRS232 < 0) 
	return -1;

    RS232_SetParameters(fdRS232, ConvertBaudrate(BaudRate), DataSize, Parity, FlowControl);
    RS232_SetStopbits(fdRS232, 1);
#else
	serial_init(BaudRate, DataSize, Parity, FlowControl);   //Only baudrate is valid
#endif
    	return 0;
}

/* Flush serial input queue. 
 * Returns 0 on success or negative error number otherwise
 */
static int arca_serial_flush_input(void)
{
#ifndef UCOS    
    tcflush(fdRS232, TCIFLUSH);
#endif
    return 0;
}

/* Flush output queue. 
 * Returns 0 on success or negative error number otherwise
 */
static int arca_serial_flush_output(void)
{
#ifndef UCOS    
    tcflush(fdRS232, TCOFLUSH);
#endif
    return 0;
}

/* Check if there is a character available to read. 
 * Returns 1 if there is a character available, 0 if not, 
 */
static int arca_serial_poll(void)
{
#ifndef UCOS
    int CharsWaiting = 0;                                                                                                            
    ioctl(fdRS232, FIONREAD, &CharsWaiting);
    
    //if (CharsWaiting >= 1) CharsWaiting = 1;
    if (CharsWaiting < 0) CharsWaiting = 0;
    
    return CharsWaiting;   
#else
	return serial_tstc();
#endif
}

/* read one character from the serial port. return character (between
 * 0 and 255) on success, or negative error number on failure. this
 * function is not blocking */
static int arca_serial_read(void)
{
#ifndef UCOS
    unsigned char TheData;
    
    if (read(fdRS232, &TheData, 1) != 1)
    {
        return -1;
    }
    return (int)TheData;
#else
	return serial_getc();
#endif
}

/* write character to serial port. return 0 on success, or negative
 * error number on failure. this function is blocking
 */
static int arca_serial_write(int c)
{
#ifndef UCOS
    unsigned char TheData;
    
    if(fdRS232==-1) return -1;
    
    TheData = c&0xff;
    
    if (write(fdRS232, &TheData, 1) != 1)
    {
        return -1;
    }
    //waits until all output written has been transmitted
#ifdef ZEM300
    //if(gOptions.RS485On) tcdrain(fdRS232);
#else
    tcdrain(fdRS232);
#endif
#else
	serial_put(c&0xff);
#endif
    return 0;
}

static int arca_serial_tcdrain(void)
{
#ifndef UCOS
    return tcdrain(fdRS232);
#else
	return 0;
#endif
}

static void arca_serial_free(void)
{
#ifndef UCOS
    if (fdRS232 > 0) close(fdRS232);
#endif
	return;
}

/* export serial driver */
serial_driver_t ff232 = {
	arca_serial_init,
	arca_serial_read,
	arca_serial_write,
	arca_serial_poll,
	arca_serial_flush_input,
	arca_serial_flush_output,
	arca_serial_tcdrain,
	arca_serial_free
};

// Initialise serial port at the request baudrate. 
static int arca0_serial_init(int BaudRate, int DataSize, int Parity, int FlowControl)
{  /* 
    if (fdRS0232 > 0) close(fdRS0232);
    fdRS0232 = open("/dev/ttyUART0", O_RDWR | O_NOCTTY);
    if ( fdRS0232 < 0) 
	    return -1;
 	   
    RS232_SetParameters(fdRS0232, ConvertBaudrate(BaudRate), DataSize, Parity, FlowControl);
    RS232_SetStopbits(fdRS0232, 1);
    return 0;*/
}

/* Flush serial input queue. 
 * Returns 0 on success or negative error number otherwise
 */
static int arca0_serial_flush_input(void)
{    
    tcflush(fdRS0232, TCIFLUSH);
    return 0;
}

/* Flush output queue. 
 * Returns 0 on success or negative error number otherwise
 */
static int arca0_serial_flush_output(void)
{
    //tcflush(fdRS0232, TCOFLUSH);
   // DelayUS(1000);
    return 0;
}

/* Check if there is a character available to read. 
 * Returns 1 if there is a character available, 0 if not, 
 * and negative error number on failure.
 */
static int arca0_serial_poll(void)
{
    int CharsWaiting = 0;
                                                                                                               
    ioctl(fdRS0232, FIONREAD, &CharsWaiting);
    
    if (CharsWaiting < 0) CharsWaiting = 0;
    
    return CharsWaiting;   
}

/* read one character from the serial port. return character (between
 * 0 and 255) on success, or negative error number on failure. this
 * function is not blocking */
static int arca0_serial_read(void)
{
    unsigned char TheData;
                                                                                                               
    if (read(fdRS0232, &TheData, 1) != 1)
    {
        return -1;
    }
    return (int)TheData;
}

/* write character to serial port. return 0 on success, or negative
 * error number on failure. this function is blocking
 */
static int arca0_serial_write(int c)
{
    unsigned char TheData;
    
    TheData = c&0xff;
    
    if (write(fdRS0232, &TheData, 1) != 1)
    {
        return -1;
    }
    
    return 0;
}

static int arca0_serial_tcdrain(void)
{
    return tcdrain(fdRS0232);
}

static void arca0_serial_free(void)
{
    if (fdRS0232 > 0) close(fdRS0232);
}

/* export serial driver */
serial_driver_t st232 = {
	arca0_serial_init,
	arca0_serial_read,
	arca0_serial_write,
	arca0_serial_poll,
	arca0_serial_flush_input,
	arca0_serial_flush_output,
	arca0_serial_tcdrain,
	arca0_serial_free
};

// Initialise serial port at the request baudrate. 
static int arca1_serial_init(int BaudRate, int DataSize, int Parity, int FlowControl)
{/*   
	if (fdRS1232 > 0) close(fdRS1232);
	fdRS1232 = open("/dev/ttyS0", O_RDWR | O_NOCTTY);
	if ( fdRS1232 < 0) 
		return -1;
	
	RS232_SetParameters(fdRS1232, ConvertBaudrate(BaudRate), DataSize, Parity, FlowControl);
	RS232_SetStopbits(fdRS1232, 1);
	return 0;*/
}

/* Flush serial input queue. 
 * Returns 0 on success or negative error number otherwise
 */
static int arca1_serial_flush_input(void)
{    
	tcflush(fdRS1232, TCIFLUSH);
	return 0;
}

/* Flush output queue. 
 * Returns 0 on success or negative error number otherwise
 */
static int arca1_serial_flush_output(void)
{
	//tcflush(fdRS1232, TCOFLUSH);
	//DelayUS(1000);
	return 0;
}

/* Check if there is a character available to read. 
 * Returns 1 if there is a character available, 0 if not, 
 * and negative error number on failure.
 */
static int arca1_serial_poll(void)
{
	int CharsWaiting = 0;
	
	ioctl(fdRS1232, FIONREAD, &CharsWaiting);
	
	if (CharsWaiting < 0) CharsWaiting = 0;
	
	return CharsWaiting;   
}

/* read one character from the serial port. return character (between
 * 0 and 255) on success, or negative error number on failure. this
 * function is not blocking */
static int arca1_serial_read(void)
{
	unsigned char TheData;
	
	if (read(fdRS1232, &TheData, 1) != 1)
	{
		return -1;
	}
	return (int)TheData;
}

/* write character to serial port. return 0 on success, or negative
 * error number on failure. this function is blocking
 */
static int arca1_serial_write(int c)
{
	unsigned char TheData;
	
	TheData = c&0xff;
	
	if (write(fdRS1232, &TheData, 1) != 1)
	{
		return -1;
	}
	
	return 0;
}

static int arca1_serial_tcdrain(void)
{
	return tcdrain(fdRS1232);
}

static void arca1_serial_free(void)
{
	if (fdRS1232 > 0) close(fdRS1232);
}

/* export serial driver */
serial_driver_t ttl232 = {
	arca1_serial_init,
	arca1_serial_read,
	arca1_serial_write,
	arca1_serial_poll,
	arca1_serial_flush_input,
	arca1_serial_flush_output,
	arca1_serial_tcdrain,
	arca1_serial_free
};

//工作方式:
//平时RS485工作在接收模式,收到主机发送的数据包后,验证后才进入发送模式,
//请在判断FIFO发送缓冲区空后返回接收模式 
void RS485_ChangeStatus(serial_driver_t *serial, U32 SendMode)
{
//	GPIO_RS485_Status(SendMode);
	if (SendMode)
		serial->flush_input();  // RS485 Send mode
	else
		serial->flush_output(); // RS485 Receive mode
	//DelayUS(1000);
	udelay(1000);
}

void RS485_setmode(U32 SendMode)
{
	RS485_ChangeStatus(&ff232, SendMode);
}

/*
 * Write a null terminated string to the serial port.
 */
void SerialOutputString(serial_driver_t *serial, const char *s)
{
	while(*s != 0)
		serial->write(*s++);
} /* SerialOutputString */
void SerialOutputData(serial_driver_t *serial, char *s,int  _size_)
{
        char *p=s;
        int size=_size_;
        while(size--) (serial)->write(*p++);
}


int SerialInputString(serial_driver_t *serial, char *s, const int len, const int timeout)
{
	U32 startTime, currentTime;
	int c;
	int i;
	int numRead;
	int skipNewline = 1;
	int maxRead = len - 1;
	
	startTime = clock();

	for(numRead = 0, i = 0; numRead < maxRead;){
		/* try to get a byte from the serial port */
		while(serial->poll() == 0){
			currentTime = clock();

			/* check timeout value */
			if((currentTime - startTime) > (timeout * CLOCKS_PER_SEC)){
				/* timeout */
				s[i++] = '\0';
				return(numRead);
			}
		}
		c = serial->read();

		/* check for errors */
		if(c < 0) {
			s[i++] = '\0';
			return c;
		}
		serial->write(c);
		/* eat newline characters at start of string */
		if((skipNewline == 1) && (c != '\r') && (c != '\n'))
			skipNewline = 0;

		if(skipNewline == 0) {
			if((c == '\r') || (c == '\n')) {
				s[i++] = '\0';
				return(numRead);
			} else {
				s[i++] = (char)c;
				numRead++;
			}
		}
	}

	return(numRead);
}

/*
void SerialPrintf(serial_driver_t *serial, char * fmt, ...)
{
    char buffer[256];
    va_list ap;

    va_start(ap,fmt);
    doPrint(buffer,fmt,ap);
    SerialOutputString(serial, buffer);
}
*/
#endif

#define O_RDWR_Y          02

#define O_CREAT_Y         0100

#define S_IREAD_Y         0000400

#define S_IWRITE_Y        0000200

void Updatefile(void)
{
	int fd=-1;
	unsigned char val;
	int fsize;
	int i,b;
	char buf[200];
//	char fbuf[1024*1024*2];

	if (ff232.init(57600, V10_8BIT, V10_NONE, 0)!=0)
		printf("init rs232 failed\n");
	else	
		printf("init rs232 success\n\n");
	
	
	int L = 'E';

//	JZ_StopTicker();
//	OSSchedLock();
	while(1)
	{
	//	printf("waiting ...\n");
		while(ff232.poll()==0);
//		while(serial_tstc()==0);
//		val=serial_getc();
		val=ff232.read();
		printf("the received byte=%x\n",val);
		
		b=0;	
		switch(val)
		{
			case 0xc0:
				sprintf(buf,"/mnt/mtdblock/%c_0.wav",L);
				break;
			case 0xc1:
				sprintf(buf,"/mnt/mtdblock/%c_1.wav",L);
				break;
			case 0xc2:
				sprintf(buf,"/mnt/mtdblock/%c_2.wav",L);
				break;
			case 0xc3:
				sprintf(buf,"/mnt/mtdblock/%c_3.wav",L);
				break;
			case 0xc4:
				sprintf(buf,"/mnt/mtdblock/%c_4.wav",L);
				break;
			case 0xc5:
				sprintf(buf,"/mnt/mtdblock/%c_5.wav",L);
				break;
			case 0xc6:
				sprintf(buf,"/mnt/mtdblock/%c_6.wav",L);
				break;
			case 0xc7:
				sprintf(buf,"/mnt/mtdblock/%c_7.wav",L);
				break;
			case 0xc8:
				sprintf(buf,"/mnt/mtdblock/%c_8.wav",L);
				break;
			case 0xc9:
				sprintf(buf,"/mnt/mtdblock/%c_9.wav",L);
				break;
			case 0xca:
				sprintf(buf,"/mnt/mtdblock/%c_10.wav",L);
				break;
			case 0xcb:
				sprintf(buf,"/mnt/mtdblock/%c_11.wav",L);
				break;
			case 0xb0:
				sprintf(buf,"/mnt/mtdblock/beep.wav");
				break;
			case 0xcc:
				sprintf(buf,"/mnt/mtdblock/options.cfg");
				break;
			case 0xcd:
				sprintf(buf,"/mnt/mtdblock/GB2312.FT");
				break;
			case 0xce:
				sprintf(buf,"/mnt/mtdblock/SYM.FT");
				break;
			case 0xcf:
				sprintf(buf,"/mnt/mtdblock/ffiso.dat");
				break;
			case 0xd0:
				sprintf(buf,"/mnt/mtdblock/LANGUAGE.E");
				break;
			case 0xd1:
				sprintf(buf,"/mnt/mtdblock/LANGUAGE.S");
				break;
			case 0xe0:	/* for testing rs232 only */
				b=2;
				sprintf(buf,"/mnt/mtdblock/test");
				break;
			case 0xff:
				return;
			default:
				b=-1; 
				break;
		}
		if(b==0||b==2)
		{
			fsize=0;
			for(i=3;i>=0;i--)
			{
				while(ff232.poll()==0);
				val=ff232.read();
				fsize |= val<<(i*8);
			}
			printf("the size of %s = %d\n",buf,fsize);
		//	yaffs_unlink(buf);
//			fbuf=mem_malloc(fsize);
			fd = yaffs_open(buf, O_CREAT | O_RDWR, S_IREAD | S_IWRITE);		
			if(fd<0)
				printf("yaffs open file failed\n");
			else
			{
				printf("yaffs open file success\n");
				yaffs_lseek(fd,0,SEEK_END);
				i=0;
				while(i<fsize)
				{
					if(ff232.poll()!=0)
						fbuf[i++]=ff232.read();
				}
				if(b!=2)
					yaffs_write(fd,fbuf,fsize);
				yaffs_close(fd);
	//			mem_free(fbuf);
				printf("finished saving %s\n",buf);
			}
		}
	}//while
//	OSSchedUnlock();
//	JZ_StartTicker(OS_TICKS_PER_SEC);
	printf("finished receiving file by serial port\n");
	return;
}

BOOL ReadOneLines(int fd, char *dest, int *size)
{
       char c;

       *size=0;
       while(TRUE)
       {
               if (yaffs_read(fd, &c, 1) == 1)
               {
                       if((c == '\n') || (c == '\r') || (c == '"'))
                       {
                               if(*size==0)
                                       continue;
                               else
                                       break;
                       }
                       dest[*size] = c;
                       *size = *size + 1;
               }
               else
                       break;
       }
       if (*size > 0)
       {
               dest[*size] = '\0';
       }
       return(*size > 0);
}

void ReadOptions(char *buf)
{
	int fd=-1;
	char buffer[1024];
	char bf[200];
	int size;

	fd = yaffs_open(buf,O_RDWR , S_IREAD | S_IWRITE);		
//	fd = open(buf, O_RDWR);		
//	fd = open(buf, O_RDWR, S_IREAD | S_IWRITE);		
	if(fd<0)
	{	printf("open file %s failed\n",buf);
		return;
	}
#if 0
	/*write testing  */
//	yaffs_read(fd, bf,100);
//	printf("reading data from file ****************\n");//treckle
//	mdelay(1000);
	sprintf(buffer,"write one line to options.cfg");
//	yaffs_lseek(fd, 0, SEEK_SET);
	yaffs_lseek(fd, 0, SEEK_END);
	yaffs_write(fd,buffer,strlen(buffer));
	printf("finished write data\n");
	yaffs_lseek(fd, 0, SEEK_SET);
	ReadOneLines(fd,buffer,&size);
	printf("the read data =%s\n",buffer);
#endif

	yaffs_lseek(fd, 0, SEEK_SET);
	while(1)
	{
		if(ReadOneLines(fd, buffer, &size))
	//	if(read(fd, buffer, 1024))
		//;
			printf(" %s\n",buffer);
		else 
		{
			printf("finished reading %s\n",buf);
			break;	
		}
	}

	close(fd);
}

/*
void TestSerial(void)
{
	char buf[50];
	int rdnum;

	RS485_setmode(TRUE);
	SerialOutputString(&ff232,"\r\nInput string 'String 101':");
	DelayUS(1000*20);
	RS485_setmode(FALSE);
	rdnum=SerialInputString(&ff232, buf, 50, 5);
	RS485_setmode(TRUE);
	if(rdnum==strlen("String 101"))
	{
		if(strcmp("String 101",buf)==0)
		{
			SerialOutputString(&ff232, "\r\n\tSerial port is OK!\r\n");
		}
		else
		{
			SerialPrintf(&ff232, "\r\n\tError: Serial port input is '%s'\r\n", buf);
		}
	}
	else if(rdnum)
	{
		SerialPrintf(&ff232, "\r\n\tError: Serial port input is '%s'\r\n", buf);
	}
	else
		SerialOutputString(&ff232, "\r\nNo input!\r\n");
}
*/
