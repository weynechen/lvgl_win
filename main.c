#include <windows.h>
#include "lvgl.h"
#include "demo.h"

static HDC hdc = NULL;
static int PosX = 0;
static int PosY = 0;
static bool IsClicked = false;

#define PAINT_AREA_X 		(LV_HOR_RES+25)
#define PAINT_AREA_Y        (LV_VER_RES + 45)

 /* Flush the content of the internal buffer the specific area on the display
  * You can use DMA or any hardware acceleration to do this operation in the background but
  * 'lv_flush_ready()' has to be called when finished
  * This function is required only when LV_VDB_SIZE != 0 in lv_conf.h*/
static void ex_disp_flush(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t * color_p)
{
	/*The most simple case (but also the slowest) to put all pixels to the screen one-by-one*/

	int32_t x;
	int32_t y;
	for (y = y1; y <= y2; y++) {
		for (x = x1; x <= x2; x++) {
			/* Put a pixel to the display. For example: */
			/* put_px(x, y, *color_p)*/
			color_p++;
			SetPixel(hdc, x, y, RGB(color_p->red, color_p->green, color_p->blue));
		}
	}

	/* IMPORTANT!!!
	 * Inform the graphics library that you are ready with the flushing*/
	lv_flush_ready();
}


/* Write a pixel array (called 'map') to the a specific area on the display
 * This function is required only when LV_VDB_SIZE == 0 in lv_conf.h*/
static void ex_disp_map(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t * color_p)
{
	/*The most simple case (but also the slowest) to put all pixels to the screen one-by-one*/

	int32_t x;
	int32_t y;
	for (y = y1; y <= y2; y++) {
		for (x = x1; x <= x2; x++) {
			/* Put a pixel to the display. For example: */
			/* put_px(x, y, *color_p)*/
			color_p++;
			SetPixel(hdc, x, y, RGB(color_p->red, color_p->green, color_p->blue));


		}
	}
}


/* Write a pixel array (called 'map') to the a specific area on the display
 * This function is required only when LV_VDB_SIZE == 0 in lv_conf.h*/
static void ex_disp_fill(int32_t x1, int32_t y1, int32_t x2, int32_t y2, lv_color_t color)
{
	/*The most simple case (but also the slowest) to put all pixels to the screen one-by-one*/

	RECT rect = { x1,y1,x2,y2 };
	HBRUSH br= CreateSolidBrush(color.full);
	FillRect(hdc, &rect, br);
	DeleteObject(br);
}

#if USE_LV_GPU
/* If your MCU has hardware accelerator (GPU) then you can use it to blend to memories using opacity
 * It can be used only in buffered mode (LV_VDB_SIZE != 0 in lv_conf.h)*/
static void ex_mem_blend(lv_color_t * dest, const lv_color_t * src, uint32_t length, lv_opa_t opa)
{
	/*It's an example code which should be done by your GPU*/

	int32_t i;
	for (i = 0; i < length; i++) {
		dest[i] = lv_color_mix(dest[i], src[i], opa);
	}
}


/* If your MCU has hardware accelerator (GPU) then you can use it to fill a memory with a color
 * It can be used only in buffered mode (LV_VDB_SIZE != 0 in lv_conf.h)*/
static void ex_mem_fill(lv_color_t * dest, uint32_t length, lv_color_t color)
{
	/*It's an example code which should be done by your GPU*/

	int32_t i;
	for (i = 0; i < length; i++) {
		dest[i] = color;
	}
}
#endif

/* Read the touchpad and store it in 'data'
 * REaturn false if no more data read; true for ready again */
static bool ex_tp_read(lv_indev_data_t *data)
{
	/* Read your touchpad */
	if (IsClicked)
	{
		IsClicked = false;
		data->state = LV_INDEV_STATE_PR;
	}
	else
	{
		data->state = LV_INDEV_STATE_REL;
	}

	data->point.x = PosX;
	data->point.y = PosY;

	return false;   /*false: no more data to read because we are no buffering*/
}

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    PSTR szCmdLine,
    int iCmdShow
){
    static WCHAR szClassName[] = TEXT("lvgl");  
    HWND     hwnd;  
    MSG      msg;  
    WNDCLASS wndclass;  
   

	lv_init();
	lv_disp_drv_t disp_drv;                         /*Descriptor of a display driver*/
	lv_disp_drv_init(&disp_drv);                    /*Basic initialization*/

	/*Set up the functions to access to your display*/
	disp_drv.disp_flush = ex_disp_flush;            /*Used in buffered mode (LV_VDB_SIZE != 0  in lv_conf.h)*/
	disp_drv.disp_fill = ex_disp_fill;              /*Used in unbuffered mode (LV_VDB_SIZE == 0  in lv_conf.h)*/
	disp_drv.disp_map = ex_disp_map;                /*Used in unbuffered mode (LV_VDB_SIZE == 0  in lv_conf.h)*/
#if USE_LV_GPU
/*Optionally add functions to access the GPU. (Only in buffered mode, LV_VDB_SIZE != 0)*/
	disp_drv.mem_blend = ex_mem_blend;              /*Blend two color array using opacity*/
	disp_drv.mem_fill = ex_mem_fill;                /*Fill a memory array with a color*/
#endif

	/*Finally register the driver*/
	lv_disp_drv_register(&disp_drv);

	/*touchpad_init();*/                            /*Initialize your touchpad*/
	lv_indev_drv_t indev_drv;                       /*Descriptor of an input device driver*/
	lv_indev_drv_init(&indev_drv);                  /*Basic initialization*/
	indev_drv.type = LV_INDEV_TYPE_POINTER;         /*The touchpad is pointer type device*/
	indev_drv.read = ex_tp_read;                 /*Library ready your touchpad via this function*/
	lv_indev_drv_register(&indev_drv);              /*Finally register the driver*/

    
    wndclass.style = CS_HREDRAW | CS_VREDRAW;  
    wndclass.lpfnWndProc  = WndProc;  
    wndclass.cbClsExtra   = 0;  
    wndclass.cbWndExtra   = 0;  
    wndclass.hInstance    = hInstance;  
    wndclass.hIcon        = LoadIcon (NULL, IDI_APPLICATION);  
    wndclass.hCursor      = LoadCursor (NULL, IDC_ARROW);  
    wndclass.hbrBackground= (HBRUSH) GetStockObject (WHITE_BRUSH);  
    wndclass.lpszMenuName = NULL ;  
    wndclass.lpszClassName= szClassName;  
    
    RegisterClass(&wndclass);
    
    hwnd = CreateWindow(
        szClassName,  
        TEXT("lvgl win demo"),  
        WS_OVERLAPPEDWINDOW,  
        CW_USEDEFAULT,  
        CW_USEDEFAULT,  
		PAINT_AREA_X,
		PAINT_AREA_Y,
        NULL,  
        NULL,  
        hInstance,  
        NULL  
    );

	PAINTSTRUCT ps;
	hdc = BeginPaint(hwnd, &ps);
    ShowWindow (hwnd, iCmdShow);
    UpdateWindow (hwnd);
    
	demo_create();

	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			lv_task_handler();
			lv_tick_inc(1);
		}
	}
	
	EndPaint(hwnd, &ps);
    return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam){
	PAINTSTRUCT ps;

    switch (message){
        case WM_PAINT:
			hdc = BeginPaint(hwnd, &ps);
            return 0 ;
        
        case WM_DESTROY:
            PostQuitMessage(0) ;
            return 0 ;

		case WM_LBUTTONDOWN:
			PosX = LOWORD(lParam);
			PosY = HIWORD(lParam);
			IsClicked = true;
			return 0;
    }
    return DefWindowProc(hwnd, message, wParam, lParam) ;
}