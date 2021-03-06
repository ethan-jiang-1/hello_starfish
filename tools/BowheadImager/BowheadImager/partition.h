#ifndef PARTITION_H
#define PARTITION_H


#define waveform_p 1
#define waveform_1 waveform_p 
#define waveform_2 waveform_p 
#define waveform_3 waveform_p 
#define waveform_4 waveform_p 
#define waveform_5 waveform_p 
#define waveform_6 waveform_p 
#define waveform_7 waveform_p 
#define waveform_8 waveform_p 
#define waveform_9 waveform_p 
#define waveform_10 waveform_p 
#define waveform_11 waveform_p 
#define waveform_12 waveform_p 
#define waveform_13 waveform_p 
#define waveform_14 waveform_p 
#define waveform_15 waveform_p 
#define waveform_16 waveform_p 
#define waveform_17 waveform_p 
#define waveform_18 waveform_p 
#define waveform_19 waveform_p 
#define waveform_20 waveform_p 
#define waveform_21 waveform_p 
#define waveform_22 waveform_p 
#define waveform_23 waveform_p 
//#define waveform_NONE	1

//#define image_p 0xff
#define image_p 1
//#define image_p 2

#define image_1 image_p 
//#define image_2 image_p 
//#define image_NONE	1


#define image2_p 0xff
//#define image2_p 1
//#define image2_p 2
//#define image2_p 3
//#define image2_p 4
//#define image2_p 5
//#define image2_p 6
//#define image2_p 7
//#define image2_p 8
//#define image2_p 9
//#define image2_p 10
//#define image2_p 11
//#define image2_p 12
//#define image2_p 13
//#define image2_p 14
//#define image2_p 15
//#define image2_p 16
//#define image2_p 17
//#define image2_p 18
//#define image2_p 19
//#define image2_p 20
//#define image2_p 21
//#define image2_p 22
//#define image2_p 23

//#define image2_1 image2_p 
//#define image2_2 image2_p 
//#define image2_3 image2_p 
//#define image2_4 image2_p 
//#define image2_5 image2_p 
//#define image2_6 image2_p 
//#define image2_7 image2_p 
//#define image2_8 image2_p 
//#define image2_9 image2_p 
//#define image2_10 image2_p 
//#define image2_11 image2_p 
//#define image2_12 image2_p 
//#define image2_13 image2_p 
//#define image2_14 image2_p 
//#define image2_15 image2_p 
//#define image2_16 image2_p 
//#define image2_17 image2_p 
//#define image2_18 image2_p 
//#define image2_19 image2_p 
//#define image2_20 image2_p
//#define image2_21 image2_p 
//#define image2_22 image2_p 
//#define image2_23 image2_p 
//#define image2_NONE	1

#define image3_p 0xff
//#define image3_p 1
//#define image3_p 2
//#define image3_p 3
//#define image3_p 4
//#define image3_p 5


//#define image3_1 image3_p 
//#define image3_2 image3_p
//#define image3_3 image3_p 
//#define image3_4 image3_p 
//#define image3_5 image3_p 
//#define image3_NONE	1



#define FIRMWARE_OFFSET			(0x100)
#define CMDSET_LENGTH				(2348)
#define WAVEFORM_LENGTH			(70808)
#define CMDSET_START_ADDR		(16 + FIRMWARE_OFFSET)
#define WAVFORM_START_ADDR	(CMDSET_START_ADDR + CMDSET_LENGTH)

#define IMAGE_START_ADD      0xB4000//(WAVFORM_START_ADDR+WAVEFORM_LENGTH)
#define IMAGE_LENGTH         (108000) 
#define IMAGE_SPACE_IN_FLASH (0x1B000)
#define IMAGE_ADDR_1     (IMAGE_START_ADD)
#define IMAGE_ADDR_2     (IMAGE_ADDR_1+IMAGE_SPACE_IN_FLASH)
#define IMAGE_ADDR_3     (IMAGE_ADDR_2+IMAGE_SPACE_IN_FLASH)
#define IMAGE_ADDR_4     (IMAGE_ADDR_3+IMAGE_SPACE_IN_FLASH)
#define IMAGE_ADDR_5     (IMAGE_ADDR_4+IMAGE_SPACE_IN_FLASH)
#define IMAGE_ADDR_6     (IMAGE_ADDR_5+IMAGE_SPACE_IN_FLASH)


  




#endif
