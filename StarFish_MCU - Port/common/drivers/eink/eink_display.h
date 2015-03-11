#ifndef _EINK_DISPLAY_H
#define _EINK_DISPLAY_H



#define BUS_TO_LCM	1
#define BUS_TO_SPI	0



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

