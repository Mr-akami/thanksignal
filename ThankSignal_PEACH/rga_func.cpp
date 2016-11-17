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
#include "Images/BinaryImage_RZ_A1H.h"

#define ZOOM_SRC_CENTER_X                   (IMAGE_WIDTH_ZOOM_FUNC / 2)
#define ZOOM_SRC_CENTER_Y                   (IMAGE_HEIGHT_ZOOM_FUNC / 2)

#define CRYSTAL_DIAMETER                    (55)

/*ループ回数を設定。使う枚数と同じ数にする。*/

Canvas2D_ContextClass canvas2d;

static animation_timing_function_t* accelerator;
Serial pc(USBTX, USBRX); // tx, rx

/*テーブルをふやそう*/
/*画像テーブルを設定*/


static const graphics_image_t* ThankSignal1[47] = {
ThankSignal1_00000,
ThankSignal1_00001,
ThankSignal1_00002,
ThankSignal1_00003,
ThankSignal1_00004,
ThankSignal1_00005,
ThankSignal1_00006,
ThankSignal1_00007,
ThankSignal1_00008,
ThankSignal1_00009,
ThankSignal1_00010,
ThankSignal1_00011,
ThankSignal1_00012,
ThankSignal1_00013,
ThankSignal1_00014,
ThankSignal1_00015,
ThankSignal1_00016,
ThankSignal1_00017,
ThankSignal1_00018,
ThankSignal1_00019,
ThankSignal1_00020,
ThankSignal1_00021,
ThankSignal1_00022,
ThankSignal1_00023,
ThankSignal1_00024,
ThankSignal1_00025,
ThankSignal1_00026,
ThankSignal1_00027,
ThankSignal1_00028,
ThankSignal1_00029,
ThankSignal1_00030,
ThankSignal1_00031,
ThankSignal1_00032,
ThankSignal1_00033,
ThankSignal1_00034,
ThankSignal1_00035,
ThankSignal1_00036,
ThankSignal1_00037,
ThankSignal1_00038,
ThankSignal1_00039,
ThankSignal1_00040,
ThankSignal1_00041,
ThankSignal1_00042,
ThankSignal1_00043,
ThankSignal1_00044,
ThankSignal1_00045,
ThankSignal1_00046
};

static const graphics_image_t* ThankSignal2[38] = {
ThankSignal2_00000,
ThankSignal2_00001,
ThankSignal2_00002,
ThankSignal2_00003,
ThankSignal2_00004,
ThankSignal2_00005,
ThankSignal2_00006,
ThankSignal2_00007,
ThankSignal2_00008,
ThankSignal2_00009,
ThankSignal2_00010,
ThankSignal2_00011,
ThankSignal2_00012,
ThankSignal2_00013,
ThankSignal2_00014,
ThankSignal2_00015,
ThankSignal2_00016,
ThankSignal2_00017,
ThankSignal2_00018,
ThankSignal2_00019,
ThankSignal2_00020,
ThankSignal2_00021,
ThankSignal2_00022,
ThankSignal2_00023,
ThankSignal2_00024,
ThankSignal2_00025,
ThankSignal2_00026,
ThankSignal2_00027,
ThankSignal2_00028,
ThankSignal2_00029,
ThankSignal2_00030,
ThankSignal2_00031,
ThankSignal2_00032,
ThankSignal2_00033,
ThankSignal2_00034,
ThankSignal2_00035,
ThankSignal2_00036,
ThankSignal2_00037
};


static const graphics_image_t* ThankSignal3[45] = {
ThankSignal3_00000,
ThankSignal3_00001,
ThankSignal3_00002,
ThankSignal3_00003,
ThankSignal3_00004,
ThankSignal3_00005,
ThankSignal3_00006,
ThankSignal3_00007,
ThankSignal3_00008,
ThankSignal3_00009,
ThankSignal3_00010,
ThankSignal3_00011,
ThankSignal3_00012,
ThankSignal3_00013,
ThankSignal3_00014,
ThankSignal3_00015,
ThankSignal3_00016,
ThankSignal3_00017,
ThankSignal3_00018,
ThankSignal3_00019,
ThankSignal3_00020,
ThankSignal3_00021,
ThankSignal3_00022,
ThankSignal3_00023,
ThankSignal3_00024,
ThankSignal3_00025,
ThankSignal3_00026,
ThankSignal3_00027,
ThankSignal3_00028,
ThankSignal3_00029,
ThankSignal3_00030,
ThankSignal3_00031,
ThankSignal3_00032,
ThankSignal3_00033,
ThankSignal3_00034,
ThankSignal3_00035,
ThankSignal3_00036,
ThankSignal3_00037,
ThankSignal3_00038,
ThankSignal3_00039,
ThankSignal3_00040,
ThankSignal3_00041,
ThankSignal3_00042,
ThankSignal3_00043,
ThankSignal3_00044
};

static const graphics_image_t* ThankSignal5[56] = {
ThankSignal5_00000,
ThankSignal5_00001,
ThankSignal5_00002,
ThankSignal5_00003,
ThankSignal5_00004,
ThankSignal5_00005,
ThankSignal5_00006,
ThankSignal5_00007,
ThankSignal5_00008,
ThankSignal5_00009,
ThankSignal5_00010,
ThankSignal5_00011,
ThankSignal5_00012,
ThankSignal5_00013,
ThankSignal5_00014,
ThankSignal5_00015,
ThankSignal5_00016,
ThankSignal5_00017,
ThankSignal5_00018,
ThankSignal5_00019,
ThankSignal5_00020,
ThankSignal5_00021,
ThankSignal5_00022,
ThankSignal5_00023,
ThankSignal5_00024,
ThankSignal5_00025,
ThankSignal5_00026,
ThankSignal5_00027,
ThankSignal5_00028,
ThankSignal5_00029,
ThankSignal5_00030,
ThankSignal5_00031,
ThankSignal5_00032,
ThankSignal5_00033,
ThankSignal5_00034,
ThankSignal5_00035,
ThankSignal5_00036,
ThankSignal5_00037,
ThankSignal5_00038,
ThankSignal5_00039,
ThankSignal5_00040,
ThankSignal5_00041,
ThankSignal5_00042,
ThankSignal5_00043,
ThankSignal5_00044,
ThankSignal5_00045,
ThankSignal5_00046,
ThankSignal5_00047,
ThankSignal5_00048,
ThankSignal5_00049,
ThankSignal5_00050,
ThankSignal5_00051,
ThankSignal5_00052,
ThankSignal5_00053,
ThankSignal5_00054,
ThankSignal5_00055
};


void Set_RGAObject(frame_buffer_t* frmbuf_info) {
    errnum_t err;
    Canvas2D_ContextConfigClass config;

    config.frame_buffer = frmbuf_info;
    canvas2d = R_RGA_New_Canvas2D_ContextClass(config);
    err = R_OSPL_GetErrNum();
    if (err != 0) {
        printf("Line %d, error %d\n", __LINE__, err);
        while (1);
    }
}

void RGA_Func_DrawTopScreen(frame_buffer_t* frmbuf_info) {
    /* Clear */
    canvas2d.clearRect(0, 0, frmbuf_info->width, frmbuf_info->height);

    /* Draw a image */
    canvas2d.drawImage(BaseImage, 0, 0);

    /* Complete drawing */
    R_GRAPHICS_Finish(canvas2d.c_LanguageContext);
}



void RGA_Func_DrawImage(frame_buffer_t* frmbuf_info, int x, int y,int message) {
    static int lcount=0;

    /* Clear */
    canvas2d.clearRect(0, 0, frmbuf_info->width, frmbuf_info->height);

    canvas2d.drawImage(ThankSignal1[lcount],0,0);
    lcount++;
    wait_ms(20);
    if(lcount > FRM_TIME1){
      lcount = 0;
    }
    pc.printf("lcount = %d",lcount);

    /* Complete drawing */
    R_GRAPHICS_Finish(canvas2d.c_LanguageContext);
}

void RGA_Func_DrawImage2(frame_buffer_t* frmbuf_info, int x, int y,int message) {
    static int lcount=0;

    /* Clear */
    canvas2d.clearRect(0, 0, frmbuf_info->width, frmbuf_info->height);

    canvas2d.drawImage(ThankSignal2[lcount],0,0);
    lcount++;
    wait_ms(20);
    if(lcount > FRM_TIME2){
      lcount = 0;
    }
    pc.printf("lcount = %d",lcount);

    /* Complete drawing */
    R_GRAPHICS_Finish(canvas2d.c_LanguageContext);
}

void RGA_Func_DrawImage3(frame_buffer_t* frmbuf_info, int x, int y,int message) {
    static int lcount=0;

    /* Clear */
    canvas2d.clearRect(0, 0, frmbuf_info->width, frmbuf_info->height);

    canvas2d.drawImage(ThankSignal3[lcount],0,0);
    lcount++;
    wait_ms(20);
    if(lcount > FRM_TIME3){
      lcount = 0;
    }
    pc.printf("lcount = %d",lcount);

    /* Complete drawing */
    R_GRAPHICS_Finish(canvas2d.c_LanguageContext);
}

void RGA_Func_DrawImage4(frame_buffer_t* frmbuf_info, int x, int y,int message) {
    static int lcount=0;

    /* Clear */
    canvas2d.clearRect(0, 0, frmbuf_info->width, frmbuf_info->height);

    canvas2d.drawImage(ThankSignal5[lcount],0,0);
    lcount++;
    wait_ms(20);
    if(lcount > FRM_TIME4){
      lcount = 0;
    }
    pc.printf("lcount = %d",lcount);

    /* Complete drawing */
    R_GRAPHICS_Finish(canvas2d.c_LanguageContext);
}
