#ifndef _EINK_DISPLAY_H
#define _EINK_DISPLAY_H



#define BUS_TO_LCM	1
#define BUS_TO_SPI	0

#define FIRMWARE_OFFSET			(0x100)
#define CMDSET_LENGTH				(2348)
#define WAVEFORM_LENGTH			(70808)
#define CMDSET_START_ADDR		(16 + FIRMWARE_OFFSET)
#define WAVFORM_START_ADDR	(CMDSET_START_ADDR + CMDSET_LENGTH)


#define IMAGE_START_ADD      (WAVFORM_START_ADDR+WAVEFORM_LENGTH)//0x11ed4
#define IMAGESIZE (360*600/2)  //108000 byte
#define FIRSTFILESECTOR ((IMAGE_START_ADD/4096)+1)
#define IMAGESTEPSECTOR ((IMAGESIZE/4096)+1) //26+1=27;

struct display_rect {
	unsigned short x;
	unsigned short y;
	unsigned short w;
	unsigned short h;
};


#define DATA_BUFFER_LEN  16//可以修改,但必须为4的整数倍且大于16小于256

extern unsigned char exchange_buff[];
extern int eink_init(void);
extern int eink_display_full(unsigned char pixel);
extern int eink_display_repeat(void);
extern int eink_display(struct display_rect * rect,  unsigned char is_full, int (*get_data)(int));

#endif

