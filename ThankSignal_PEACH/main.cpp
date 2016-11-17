/*
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
 
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
 
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include "mbed.h"
#include "rga_func.h"
#include "DisplayBace.h"
#include "rtos.h"
#include "LCD_shield_config_4_3inch.h"

#define GRAPHICS_FORMAT                     (DisplayBase::GRAPHICS_FORMAT_RGB565)
#define WR_RD_WRSWA                         (DisplayBase::WR_RD_WRSWA_32_16BIT)
#define TOUCH_NUM                           (1u)

/* FRAME BUFFER Parameter */
#define FRAME_BUFFER_BYTE_PER_PIXEL         (2)
#define FRAME_BUFFER_STRIDE                 (((LCD_PIXEL_WIDTH * FRAME_BUFFER_BYTE_PER_PIXEL) + 31u) & ~31u)

#define DRAW_RECTANGLE_CNT_MAX              (4)

/*動画の枚数に合わせて変更*/

//Serial pc(USBTX, USBRX);
Serial serial00(D1,D0);
Serial serial05(D6,D7);
extern Serial pc;

typedef enum {
    RGA_FUNC_NON,
    RGA_FUNC_DRAW_RECTANGLE,
    RGA_FUNC_DRAW_IMAGE,
    RGA_FUNC_DISSOLVE,
    RGA_FUNC_SCROLL,
    RGA_FUNC_ZOOM,
    RGA_FUNC_ROTATION,
    RGA_FUNC_ACCELERATE,
    RGA_FUNC_ANIME_EASE,
    RGA_FUNC_ANIME_LINEAR,
    RGA_FUNC_ANIME_EASE_IN,
    RGA_FUNC_ANIME_EASE_OUT,
    RGA_FUNC_ANIME_EASE_IN_OUT,
    RGA_FUNC_RETURN,
    RGA_FUNC_END
} func_code_t;

DigitalOut  lcd_pwon(P7_15);
DigitalOut  lcd_blon(P8_1);
PwmOut      lcd_cntrst(P8_15);
DisplayBase Display;

typedef struct {
    uint32_t pic_pos_x;             /* X position of the key picture. */
    uint32_t pic_pos_y;             /* Y position of the key picture. */
    uint32_t pic_width;             /* Width of the key picture. */
    uint32_t pic_height;            /* Height of the key picture. */
    func_code_t func_code;          /* func code of the key picture. */
} key_pic_info_t;

#if defined(__ICCARM__)
#pragma data_alignment=32
static uint8_t user_frame_buffer1[FRAME_BUFFER_STRIDE * LCD_PIXEL_HEIGHT];
static uint8_t user_frame_buffer2[FRAME_BUFFER_STRIDE * LCD_PIXEL_HEIGHT];
#pragma data_alignment=4
#else
static uint8_t user_frame_buffer1[FRAME_BUFFER_STRIDE * LCD_PIXEL_HEIGHT]__attribute((aligned(32))); /* 32 bytes aligned */
static uint8_t user_frame_buffer2[FRAME_BUFFER_STRIDE * LCD_PIXEL_HEIGHT]__attribute((aligned(32))); /* 32 bytes aligned */
#endif
static frame_buffer_t frame_buffer_info;
static volatile int32_t vsync_count = 0;

static const key_pic_info_t top_screen_key_tbl[] = {
    /*   X       Y     Width   Height   Func code                 */
    {    30,    194,     72,     31,    RGA_FUNC_DRAW_RECTANGLE    },  /* RGA Func1 */
    {   138,    194,     72,     31,    RGA_FUNC_DRAW_IMAGE        },  /* RGA Func2 */
    {   246,    194,     72,     31,    RGA_FUNC_DISSOLVE          },  /* RGA Func3 */
    {    30,    236,     72,     31,    RGA_FUNC_SCROLL            },  /* RGA Func4 */
    {   138,    236,     72,     31,    RGA_FUNC_ZOOM              },  /* RGA Func5 */
    {   246,    236,     72,     31,    RGA_FUNC_ROTATION          },  /* RGA Func6 */
    {   369,    236,     72,     31,    RGA_FUNC_ACCELERATE        },  /* RGA Func7 */
    {     0,      0,      0,      0,    RGA_FUNC_END               }   /* table end */
};

static const key_pic_info_t return_key_tbl[] = {
    /*   X       Y     Width   Height   Func code                 */
    {   384,      6,     90,     50,    RGA_FUNC_RETURN            },  /* Return Top Screen */
    {     0,      0,      0,      0,    RGA_FUNC_END               }   /* table end */
};

static const key_pic_info_t animetion_timing_key_tbl[] = {
    /*   X       Y     Width   Height   Func code                 */
    {   384,      6,     90,     50,    RGA_FUNC_RETURN            },  /* Return Top Screen */
    {    10,    207,     81,     30,    RGA_FUNC_ANIME_EASE        },  /* ease */
    {   103,    207,     81,     30,    RGA_FUNC_ANIME_LINEAR      },  /* linear */
    {   198,    207,     81,     30,    RGA_FUNC_ANIME_EASE_IN     },  /* ease-in */
    {   292,    207,     81,     30,    RGA_FUNC_ANIME_EASE_OUT    },  /* ease-out */
    {   386,    207,     81,     30,    RGA_FUNC_ANIME_EASE_IN_OUT },  /* ease-in-out */
    {     0,      0,      0,      0,    RGA_FUNC_END               }   /* table end */
};

/****** LCD ******/
static void IntCallbackFunc_LoVsync(DisplayBase::int_type_t int_type) {
    /* Interrupt callback function for Vsync interruption */
    if (vsync_count > 0) {
        vsync_count--;
    }
}

static void Wait_Vsync(const int32_t wait_count) {
    /* Wait for the specified number of times Vsync occurs */
    vsync_count = wait_count;
    while (vsync_count > 0) {
        /* Do nothing */
    }
}

static void Init_LCD_Display(void) {
    DisplayBase::graphics_error_t error;
    DisplayBase::lcd_config_t lcd_config;
    PinName lvds_pin[8] = {
        /* data pin */
        P5_7, P5_6, P5_5, P5_4, P5_3, P5_2, P5_1, P5_0
    };

    lcd_pwon = 0;
    lcd_blon = 0;
    Thread::wait(100);
    lcd_pwon = 1;
    lcd_blon = 1;

    Display.Graphics_Lvds_Port_Init(lvds_pin, 8);

    /* Graphics initialization process */
    lcd_config = LcdCfgTbl_LCD_shield;
    error = Display.Graphics_init(&lcd_config);
    if (error != DisplayBase::GRAPHICS_OK) {
        printf("Line %d, error %d\n", __LINE__, error);
        mbed_die();
    }

    /* Interrupt callback function setting (Vsync signal output from scaler 0) */
    error = Display.Graphics_Irq_Handler_Set(DisplayBase::INT_TYPE_S0_LO_VSYNC, 0, IntCallbackFunc_LoVsync);
    if (error != DisplayBase::GRAPHICS_OK) {
        printf("Line %d, error %d\n", __LINE__, error);
        mbed_die();
    }
}

static void Start_LCD_Display(uint8_t * p_buf) {
    DisplayBase::rect_t rect;

    rect.vs = 0;
    rect.vw = LCD_PIXEL_HEIGHT;
    rect.hs = 0;
    rect.hw = LCD_PIXEL_WIDTH;
    Display.Graphics_Read_Setting(
        DisplayBase::GRAPHICS_LAYER_0,
        (void *)p_buf,
        FRAME_BUFFER_STRIDE,
        GRAPHICS_FORMAT,
        WR_RD_WRSWA,
        &rect
    );
    Display.Graphics_Start(DisplayBase::GRAPHICS_LAYER_0);
}

static void Update_LCD_Display(frame_buffer_t * frmbuf_info) {
    Display.Graphics_Read_Change(DisplayBase::GRAPHICS_LAYER_0,
     (void *)frmbuf_info->buffer_address[frmbuf_info->draw_buffer_index]);
    Wait_Vsync(1);
}

static void Swap_FrameBuffer(frame_buffer_t * frmbuf_info) {
    if (frmbuf_info->draw_buffer_index == 1) {
        frmbuf_info->draw_buffer_index = 0;
    } else {
        frmbuf_info->draw_buffer_index = 1;
    }
}

/****** Touch ******/
static func_code_t Scan_Key(const key_pic_info_t * key_tbl, const uint32_t pos_x, const uint32_t pos_y) {
    func_code_t ret = RGA_FUNC_NON;

    while (ret == RGA_FUNC_NON) {
        if (key_tbl->func_code == RGA_FUNC_END) {
            break;
        }
        /* Check the range of the X position */
        if ((pos_x >= key_tbl->pic_pos_x) && (pos_x <= (key_tbl->pic_pos_x + key_tbl->pic_width))) {
            /* Check the range of the Y position */
            if ((pos_y >= key_tbl->pic_pos_y) && (pos_y <= (key_tbl->pic_pos_y + key_tbl->pic_height))) {
                /* Decide the func code. */
                ret = key_tbl->func_code;
            }
        }
        key_tbl++;
    }

    return ret;
}

static void Exe_Myimage_Func(int message, frame_buffer_t* frmbuf_info){
  static int loop_count = 0;
  int center_pos_x = 320;
  int center_pos_y = 110;

  while (1) {
    /* Get coordinates */
    if(message == 1){
      if (loop_count > FRM_TIME1) {
        loop_count = 0;
        printf("loop_count %d",loop_count);    
        break;
        }
      loop_count++;
      /* Draw screen */
      Swap_FrameBuffer(frmbuf_info);
      RGA_Func_DrawImage(frmbuf_info, center_pos_x, center_pos_y, message);
      Update_LCD_Display(frmbuf_info);
    }else if(message == 2){
        if (loop_count > FRM_TIME2) {
          loop_count = 0;
          break;
        }
        loop_count++;
        /* Draw screen */
        Swap_FrameBuffer(frmbuf_info);
        RGA_Func_DrawImage2(frmbuf_info, center_pos_x, center_pos_y,message);
        Update_LCD_Display(frmbuf_info);
    }else if(message == 3){
        if (loop_count > FRM_TIME3) {
          loop_count = 0;
          break;
        }
        loop_count++;
        /* Draw screen */
        Swap_FrameBuffer(frmbuf_info);
        RGA_Func_DrawImage3(frmbuf_info, center_pos_x, center_pos_y,message);
        Update_LCD_Display(frmbuf_info);
    }else if(message == 4){
        if (loop_count > FRM_TIME4) {
          loop_count = 0;
          break;
        }
        loop_count++;
        /* Draw screen */
        Swap_FrameBuffer(frmbuf_info);
        RGA_Func_DrawImage4(frmbuf_info, center_pos_x, center_pos_y,message);
        Update_LCD_Display(frmbuf_info);
    }
  }
}

/****** Efect ******/
static void Exe_RGA_Func(func_code_t func_name, frame_buffer_t* frmbuf_info, TouckKey_LCD_shield * p_touch) {
    uint8_t touch_num = 0;
    TouchKey::touch_pos_t touch_pos[TOUCH_NUM];

    switch (func_name) {
        case RGA_FUNC_DRAW_RECTANGLE:
            bool key_on = false;
            int cnt;
            int color_cnt = 0;
            int x_0 = 0;
            int y_0 = 0;
            draw_rectangle_pos_t pos_tbl[DRAW_RECTANGLE_CNT_MAX] = {0};

            pos_tbl[0].style = "#FF0000";  /* red */
            pos_tbl[1].style = "#00FF00";  /* green */
            pos_tbl[2].style = "#0000FF";  /* blue */
            pos_tbl[3].style = "#000000";  /* black */

            while (1) {
                /* Get coordinates */
                touch_num = p_touch->GetCoordinates(TOUCH_NUM, touch_pos);
                if (touch_num != 0) {
                    if (Scan_Key(return_key_tbl, touch_pos[0].x, touch_pos[0].y) == RGA_FUNC_RETURN) {
                        break;
                    }
                    if (key_on == false) {
                        key_on = true;
                        if (color_cnt == 0) {
                            for (cnt = 0; cnt < DRAW_RECTANGLE_CNT_MAX; cnt++) {
                                pos_tbl[cnt].x = 0;
                                pos_tbl[cnt].y = 0;
                                pos_tbl[cnt].w = 0;
                                pos_tbl[cnt].h = 0;
                            }
                        }
                        x_0 = touch_pos[0].x;
                        y_0 = touch_pos[0].y;
                    }
                    if (x_0 < touch_pos[0].x) {
                        pos_tbl[color_cnt].x = x_0;
                        pos_tbl[color_cnt].w = touch_pos[0].x - x_0;
                    } else {
                        pos_tbl[color_cnt].x = touch_pos[0].x;
                        pos_tbl[color_cnt].w = x_0 - touch_pos[0].x;
                    }
                    if (y_0 < touch_pos[0].y) {
                        pos_tbl[color_cnt].y = y_0;
                        pos_tbl[color_cnt].h = touch_pos[0].y - y_0;
                    } else {
                        pos_tbl[color_cnt].y = touch_pos[0].y;
                        pos_tbl[color_cnt].h = y_0 - touch_pos[0].y;
                    }
                } else {
                    if (key_on != false) {
                        color_cnt++;
                        if (color_cnt == DRAW_RECTANGLE_CNT_MAX) {
                            color_cnt = 0;
                        }
                    }
                    key_on = false;
                }
                /* Draw screen */
                Swap_FrameBuffer(frmbuf_info);
                RGA_Func_DrawRectangle(frmbuf_info, pos_tbl, DRAW_RECTANGLE_CNT_MAX);
                Update_LCD_Display(frmbuf_info);
            }
            break;
        case RGA_FUNC_DRAW_IMAGE:
            int center_pos_x = 320;
            int center_pos_y = 110;
            while (1) {
                /* Get coordinates */
                touch_num = p_touch->GetCoordinates(TOUCH_NUM, touch_pos);
                if (touch_num != 0) {
                    if (Scan_Key(return_key_tbl, touch_pos[0].x, touch_pos[0].y) == RGA_FUNC_RETURN) {
                        break;
                    }
                    center_pos_x = touch_pos[0].x;
                    center_pos_y = touch_pos[0].y;
                }
                /* Draw screen */
                Swap_FrameBuffer(frmbuf_info);
                //RGA_Func_DrawImage(frmbuf_info, center_pos_x, center_pos_y);
                Update_LCD_Display(frmbuf_info);
            }
            break;
        case RGA_FUNC_DISSOLVE:
            float32_t work_alpha = 0.0f;
            while (1) {
                /* Get coordinates */
                touch_num = p_touch->GetCoordinates(TOUCH_NUM, touch_pos);
                if (touch_num != 0) {
                    if (Scan_Key(return_key_tbl, touch_pos[0].x, touch_pos[0].y) == RGA_FUNC_RETURN) {
                        break;
                    }
                    work_alpha = (float32_t)touch_pos[0].x / (float32_t)(LCD_PIXEL_WIDTH);
                }
                /* Draw screen */
                Swap_FrameBuffer(frmbuf_info);
                RGA_Func_Dissolve(frmbuf_info, work_alpha);
                Update_LCD_Display(frmbuf_info);
            }
            break;
        case RGA_FUNC_SCROLL:
            int work_width_pos = 0;
            while (1) {
                /* Get coordinates */
                touch_num = p_touch->GetCoordinates(TOUCH_NUM, touch_pos);
                if (touch_num != 0) {
                    if (Scan_Key(return_key_tbl, touch_pos[0].x, touch_pos[0].y) == RGA_FUNC_RETURN) {
                        break;
                    }
                    work_width_pos = SCROLL_MAX_NUM * ((float32_t)touch_pos[0].x / (float32_t)(LCD_PIXEL_WIDTH));
                }
                /* Draw screen */
                Swap_FrameBuffer(frmbuf_info);
                RGA_Func_Scroll(frmbuf_info, work_width_pos);
                Update_LCD_Display(frmbuf_info);
            }
            break;
        case RGA_FUNC_ZOOM:
            int work_height_pos = ZOOM_MAX_NUM;
            while (1) {
                /* Get coordinates */
                touch_num = p_touch->GetCoordinates(TOUCH_NUM, touch_pos);
                if (touch_num != 0) {
                    if (Scan_Key(return_key_tbl, touch_pos[0].x, touch_pos[0].y) == RGA_FUNC_RETURN) {
                        break;
                    }
                    work_height_pos = ZOOM_MAX_NUM * ((float32_t)touch_pos[0].x / (float32_t)(LCD_PIXEL_WIDTH));
                }
                /* Draw screen */
                Swap_FrameBuffer(frmbuf_info);
                RGA_Func_Zoom(frmbuf_info,  work_height_pos);
                Update_LCD_Display(frmbuf_info);
            }
            break;
        case RGA_FUNC_ROTATION:
            graphics_matrix_float_t work_angle = 0;
            while (1) {
                /* Get coordinates */
                touch_num = p_touch->GetCoordinates(TOUCH_NUM, touch_pos);
                if (touch_num != 0) {
                    if (Scan_Key(return_key_tbl, touch_pos[0].x, touch_pos[0].y) == RGA_FUNC_RETURN) {
                        break;
                    }
                    work_angle = ROTATION_MAX_NUM * ((float32_t)touch_pos[0].x / (float32_t)(LCD_PIXEL_WIDTH));
                }
                /* Draw screen */
                Swap_FrameBuffer(frmbuf_info);
                RGA_Func_Rotation(frmbuf_info, work_angle);
                Update_LCD_Display(frmbuf_info);
            }
            break;
        case RGA_FUNC_ACCELERATE:
            int acce_frame_num = 0;
            int animation_timing = 0;
            float32_t work_relative_pos;
            while (1) {
                /* Get coordinates */
                touch_num = p_touch->GetCoordinates(TOUCH_NUM, touch_pos);
                if (touch_num != 0) {
                    func_code_t func_code;

                    func_code = Scan_Key(animetion_timing_key_tbl, touch_pos[0].x, touch_pos[0].y);
                    if (func_code == RGA_FUNC_RETURN) {
                        break;
                    }
                    switch (func_code) {
                        case RGA_FUNC_ANIME_EASE:
                            animation_timing = 0;
                            acce_frame_num = 0;
                            break;
                        case RGA_FUNC_ANIME_LINEAR:
                            animation_timing = 1;
                            acce_frame_num = 0;
                            break;
                        case RGA_FUNC_ANIME_EASE_IN:
                            animation_timing = 2;
                            acce_frame_num = 0;
                            break;
                        case RGA_FUNC_ANIME_EASE_OUT:
                            animation_timing = 3;
                            acce_frame_num = 0;
                            break;
                        case RGA_FUNC_ANIME_EASE_IN_OUT:
                            animation_timing = 4;
                            acce_frame_num = 0;
                            break;
                        default:
                            /* Do Nothing */
                            break;
                    }
                }
                work_relative_pos = acce_frame_num / (float32_t)ACCELERATE_MAX_NUM;
                acce_frame_num++;
                if (acce_frame_num > ACCELERATE_MAX_NUM) {
                    acce_frame_num = 0;
                }
                /* Draw screen */
                Swap_FrameBuffer(frmbuf_info);
                RGA_Func_Accelerate(frmbuf_info, animation_timing, work_relative_pos);
                Update_LCD_Display(frmbuf_info);
            }
            break;
        default :
            /* Do nothing */
            break;
    }
}

int main(void) {
    func_code_t func_code;
    uint8_t touch_num = 0;
    TouchKey::touch_pos_t touch_pos[TOUCH_NUM];
    int f_code;

    /* Initialization of LCD */
    Init_LCD_Display();    /* When using LCD, please call before than Init_Video(). */

    memset(user_frame_buffer1, 0, sizeof(user_frame_buffer1));
    memset(user_frame_buffer2, 0, sizeof(user_frame_buffer2));
    frame_buffer_info.buffer_address[0] = user_frame_buffer1;
    frame_buffer_info.buffer_address[1] = user_frame_buffer2;
    frame_buffer_info.buffer_count      = 2;
    frame_buffer_info.show_buffer_index = 0;
    frame_buffer_info.draw_buffer_index = 0;
    frame_buffer_info.width             = LCD_PIXEL_WIDTH;
    frame_buffer_info.byte_per_pixel    = FRAME_BUFFER_BYTE_PER_PIXEL;
    frame_buffer_info.stride            = LCD_PIXEL_WIDTH * FRAME_BUFFER_BYTE_PER_PIXEL;
    frame_buffer_info.height            = LCD_PIXEL_HEIGHT;
    frame_buffer_info.pixel_format      = PIXEL_FORMAT_RGB565;

    /* Display Top Screen */
    Set_RGAObject(&frame_buffer_info);
    RGA_Func_DrawTopScreen(&frame_buffer_info);

    /* Start of LCD */
    Start_LCD_Display(frame_buffer_info.buffer_address[0]);

    /* Backlight on */
    Thread::wait(200);
    lcd_cntrst.write(1.0);

    /* Reset touch IC */
    TouckKey_LCD_shield touch(P4_0, P2_13, I2C_SDA, I2C_SCL);
    touch.Reset();
    int message;
    //int i;
    while (1) {
        //printf("ju");
       if(serial05.readable()){
           int c = serial05.getc();
          // printf("sin");
           printf("c = %d",c);
       
            if(c != 0xFF)  {
                //return;
            }
            else {
                c = serial05.getc();
                message = c;

                Exe_Myimage_Func(message, &frame_buffer_info);
                Swap_FrameBuffer(&frame_buffer_info);
                RGA_Func_DrawTopScreen(&frame_buffer_info);
                Update_LCD_Display(&frame_buffer_info);
            //}
       }
    }
        //printf("%s\n",message);
        
        //printf("aiueo");
        /* Get Coordinates */
        
        
        Thread::wait(20);
    }
}
