/* CH376оƬ Ӳ������� V1.0 */
/* �ṩI/O�ӿ��ӳ��� */
#include	"CH376INC.h"
#include "../jz4740/clock.h"
#ifndef	__CH376_HAL_H__
#define __CH376_HAL_H__


/* ���ӵ�USB����״̬���� */
#define		ERR_USB_UNKNOWN		0xFA	/* δ֪����,��Ӧ�÷��������,����Ӳ�����߳������ */

/* ��ʱָ��΢��ʱ��,���ݵ�Ƭ����Ƶ����,����ȷ */
//void	mDelayuS( UINT8 us );
#define	mDelayuS(us)	udelay(us)

/* ��ʱָ������ʱ��,���ݵ�Ƭ����Ƶ����,����ȷ */
//void	mDelaymS( UINT8 ms );
#define	mDelaymS(ms)	mdelay(ms)

void	CH376_PORT_INIT( void );  		/* CH376ͨѶ�ӿڳ�ʼ�� */

//void	xEndCH376Cmd( void );			/* ����CH376����,������SPI�ӿڷ�ʽ */
#define xEndCH376Cmd( )   /* ����CH376����,������SPI�ӿڷ�ʽ */ \
        do{     \
        } \
        while(0)


void	xWriteCH376Cmd( UINT8 mCmd );	/* ��CH376д���� */

void	xWriteCH376Data( UINT8 mData );	/* ��CH376д���� */

UINT8	xReadCH376Data( void );			/* ��CH376������ */

UINT8	Query376Interrupt( void );		/* ��ѯCH376�ж�(INT#����Ϊ�͵�ƽ) */

UINT8	mInitCH376Host( void );			/* ��ʼ��CH376 */

#endif