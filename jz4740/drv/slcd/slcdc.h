#ifndef __SLCDC_H__
#define __SLCDC_H__

#define STFT_PSHI	(1 << 15)
#define STFT_CLSHI	(1 << 14)
#define STFT_SPLHI	(1 << 13)
#define STFT_REVHI	(1 << 12)

#define SYNC_MASTER	(0 << 16)
#define SYNC_SLAVE	(1 << 16)

#define DE_P		(0 << 9)
#define DE_N		(1 << 9)

#define PCLK_P		(0 << 10)
#define PCLK_N		(1 << 10)

#define HSYNC_P		(0 << 11)
#define HSYNC_N		(1 << 11)

#define VSYNC_P		(0 << 8)
#define VSYNC_N		(1 << 8)

#define DATA_NORMAL	(0 << 17)
#define DATA_INVERSE	(1 << 17)

#define MODE_MASK		0x0f
#define MODE_TFT_18BIT		(1 << 7)
#define MODE_8BIT_SERIAL_TFT    0x0c
#define MODE_TFT_GEN		0x00
#define MODE_TFT_SHARP		0x01
#define MODE_TFT_CASIO		0x02
#define MODE_TFT_SAMSUNG	0x03
#define MODE_CCIR656_NONINT	0x04
#define MODE_CCIR656_INT	0x05
#define MODE_STN_COLOR_SINGLE	0x08
#define MODE_STN_MONO_SINGLE	0x09
#define MODE_STN_COLOR_DUAL	0x0a
#define MODE_STN_MONO_DUAL	0x0b

#define STN_DAT_PIN1	(0x00 << 4)
#define STN_DAT_PIN2	(0x01 << 4)
#define STN_DAT_PIN4	(0x02 << 4)
#define STN_DAT_PIN8	(0x03 << 4)
#define STN_DAT_PINMASK	STN_DAT_PIN8

#ifndef u8
#define u8	unsigned char
#endif

#ifndef u16
#define u16	unsigned short
#endif

#ifndef u32
#define u32	unsigned int
#endif

#ifndef NULL
#define NULL	0
#endif

#define NR_PALETTE	256

struct jz_dma_desc {
	 u32 dcmd;	/* DCMD value for the current transfer */
	 u32 dsadr;	/* DSAR value for the current transfer */
	 u32 dtadr;	/* DTAR value for the current transfer */
	 u32 ddadr;	/* Points to the next descriptor + transfer count */
};

#define  FRAMEBUF_NUM 3

struct jzfb_slcd_info {

	u32 cfg;	/* panel mode and pin usage etc. */
	u32 w;
	u32 h;
	u32 bpp;	/* bit per pixel */
	u32 bus;
	u32 pclk;	/* frame clk */

	u8 *cpal;	/* Cacheable Palette Buffer */
	u8 *pal;	/* Non-cacheable Palette Buffer */
	u8 *cframe;	/* Cacheable Frame Buffer */
	u8 *frame;	/* Non-cacheable Frame Buffer */
	u8 *cframe2[FRAMEBUF_NUM];	/* Cacheable Frame Buffer */
	u8 *frame2[FRAMEBUF_NUM];	  /* Non-cacheable Frame Buffer */

	struct {
		u8 red, green, blue;
	} palette[NR_PALETTE];
};

extern struct jzfb_slcd_info jzfb_slcd;

extern void Mcupanel_RegSet(u32 cmd, u32 data);
extern void Mcupanel_Command(u32 cmd);

#undef LCD_UNCACHED
#define LCD_UNCACHED(addr)	((unsigned int)(addr) | 0xa0000000)


int jzlcd_init(void);

#endif /* __LCDC_H__ */

