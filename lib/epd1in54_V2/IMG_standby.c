# include "images.h"
# include <pgmspace.h>

const uint8_t IMG_standby_data[] PROGMEM = 
{
	0x00, 0x00, 0x14, 0x00, 0x00, //          ##         
	0x00, 0x00, 0x14, 0x00, 0x00, //          ##         
	0x00, 0x00, 0x14, 0x00, 0x00, //          ##         
	0x00, 0x00, 0x14, 0x00, 0x00, //          ##         
	0x00, 0x54, 0x14, 0x15, 0x00, //     ###  ##  ###    
	0x01, 0x54, 0x14, 0x15, 0x40, //    ####  ##  ####   
	0x01, 0x54, 0x14, 0x15, 0x40, //    ####  ##  ####   
	0x05, 0x50, 0x14, 0x05, 0x50, //   ####   ##   ####  
	0x05, 0x40, 0x14, 0x01, 0x50, //   ###    ##    ###  
	0x15, 0x40, 0x14, 0x01, 0x54, //  ####    ##    #### 
	0x15, 0x00, 0x14, 0x00, 0x54, //  ###     ##     ### 
	0x15, 0x00, 0x00, 0x00, 0x54, //  ###            ### 
	0x15, 0x40, 0x00, 0x01, 0x54, //  ####          #### 
	0x05, 0x40, 0x00, 0x01, 0x50, //   ###          ###  
	0x05, 0x40, 0x00, 0x01, 0x50, //   ###          ###  
	0x01, 0x50, 0x00, 0x05, 0x40, //    ###        ###   
	0x01, 0x54, 0x00, 0x15, 0x40, //    ####      ####   
	0x00, 0x55, 0x41, 0x55, 0x00, //     #####  #####    
	0x00, 0x15, 0x55, 0x54, 0x00, //      ##########     
	0x00, 0x01, 0x55, 0x40, 0x00, //        ######       
};

sIMAGE IMG_standby = {
  IMG_standby_data,
  20, /* Width */
  20, /* Height */
};
