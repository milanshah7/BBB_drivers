/***************************************

* Name          : 7seg_display.h
* Created on    : Thursday 28 January 2021 02:40:08 PM IST
* Author        : Milan Shah <mshah.opensource@gmail.com>
* Description   :

****************************************/

#define HIGH          1
#define LOW           0

#define GPIO_DIR_OUT        HIGH
#define GPIO_DIR_IN         LOW

#define GPIO_LOW_VALUE      LOW
#define GPIO_HIGH_VALUE     HIGH

#define SEGMENT_ON          HIGH
#define SEGMENT_OFF         LOW

#define MAX_SEGMENT      8

#define GPIO_66     66     // GPIO2[2]
#define GPIO_67     67     // GPIO2[3]
#define GPIO_69     69     // GPIO2[5]
#define GPIO_68     68     // GPIO2[4]
#define GPIO_45     45     // GPIO1[13]
#define GPIO_44     44     // GPIO1[12]
#define GPIO_26     26     // GPIO0[26]
#define GPIO_46     46     // GPIO1[14]

#define GPIO_P8_7_SEGA       GPIO_66
#define GPIO_P8_8_SEGB       GPIO_67
#define GPIO_P8_9_SEGC       GPIO_69
#define GPIO_P8_10_DP        GPIO_68
#define GPIO_P8_11_SEGD      GPIO_45
#define GPIO_P8_12_SEGE      GPIO_44
#define GPIO_P8_14_SEGF      GPIO_26
#define GPIO_P8_16_SEGG      GPIO_46
