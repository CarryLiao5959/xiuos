/**
 * @file lv_port_indev_templ.c
 *
 */

 /*Copy this file as "lv_port_indev.c" and set this value to "1" to enable content*/
#if 1

/*********************
 *      INCLUDES
 *********************/
#include "lv_port_indev_template.h"
#include "../lvgl.h"

#include <dev_touch.h>

static int touch_fd = 0;
/*********************
 *      DEFINES
 *********************/
#define LV_USE_INDEV_TOUCHPAD 0x1u
#define LV_USE_INDEV_MOUSE    0x2u
#define LV_USE_INDEV_KEYPAD   0x4u
#define LV_USE_INDEV_ENCODER  0x8u
#define LV_USE_INDEV_BUTTUN   0x10u

#define PRESS_FAILED_LIMIT    15
#define LV_USE_INDEV  LV_USE_INDEV_TOUCHPAD  ///< modify this DEFINE to enable the indev device. e.g #define LV_USE_INDEV  LV_USE_INDEV_TOUCHPAD | LV_USE_INDEV_KEYPAD

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
#if (LV_USE_INDEV & LV_USE_INDEV_TOUCHPAD) == LV_USE_INDEV_TOUCHPAD
static void touchpad_init(void);
static void touchpad_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data);
// static bool touchpad_is_pressed(void);
// static void touchpad_get_xy(lv_coord_t * x, lv_coord_t * y);
static bool touchpad_is_pressed(struct TouchDataStandard* touch_data_ptr);
static void touchpad_get_xy(struct TouchDataStandard* touch_data_ptr,
    lv_coord_t* x, lv_coord_t* y);
#endif

#if (LV_USE_INDEV & LV_USE_INDEV_MOUSE) == LV_USE_INDEV_MOUSE
static void mouse_init(void);
static void mouse_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data);
static bool mouse_is_pressed(void);
static void mouse_get_xy(lv_coord_t * x, lv_coord_t * y);
#endif

#if (LV_USE_INDEV & LV_USE_INDEV_KEYPAD) == LV_USE_INDEV_KEYPAD
static void keypad_init(void);
static void keypad_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data);
static uint32_t keypad_get_key(void);
#endif

#if (LV_USE_INDEV & LV_USE_INDEV_ENCODER) == LV_USE_INDEV_ENCODER
static void encoder_init(void);
static void encoder_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data);
static void encoder_handler(void);
#endif

#if (LV_USE_INDEV & LV_USE_INDEV_BUTTUN) == LV_USE_INDEV_BUTTUN
static void button_init(void);
static void button_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data);
static int8_t button_get_pressed_id(void);
static bool button_is_pressed(uint8_t id);
#endif

/**********************
 *  STATIC VARIABLES
 **********************/
#if (LV_USE_INDEV & LV_USE_INDEV_TOUCHPAD) == LV_USE_INDEV_TOUCHPAD
lv_indev_t * indev_touchpad;
#endif

#if (LV_USE_INDEV & LV_USE_INDEV_MOUSE) == LV_USE_INDEV_MOUSE
lv_indev_t * indev_mouse;
#endif

#if (LV_USE_INDEV & LV_USE_INDEV_KEYPAD) == LV_USE_INDEV_KEYPAD
lv_indev_t * indev_keypad;
#endif

#if (LV_USE_INDEV & LV_USE_INDEV_ENCODER) == LV_USE_INDEV_ENCODER
lv_indev_t * indev_encoder;
#endif

#if (LV_USE_INDEV & LV_USE_INDEV_BUTTUN) == LV_USE_INDEV_BUTTUN
lv_indev_t * indev_button;
#endif

static int32_t encoder_diff;
static lv_indev_state_t encoder_state;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_port_indev_init(void)
{
    /**
     * Here you will find example implementation of input devices supported by LittelvGL:
     *  - Touchpad
     *  - Mouse (with cursor support)
     *  - Keypad (supports GUI usage only with key)
     *  - Encoder (supports GUI usage only with: left, right, push)
     *  - Button (external buttons to press points on the screen)
     *
     *  The `..._read()` function are only examples.
     *  You should shape them according to your hardware
     */

    static lv_indev_drv_t indev_drv;
#if (LV_USE_INDEV & LV_USE_INDEV_TOUCHPAD) == LV_USE_INDEV_TOUCHPAD
    /*------------------
     * Touchpad
     * -----------------*/

    /*Initialize your touchpad if you have*/
    touchpad_init();

    /*Register a touchpad input device*/
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = touchpad_read;
    indev_touchpad = lv_indev_drv_register(&indev_drv);
#endif

#if (LV_USE_INDEV & LV_USE_INDEV_MOUSE) == LV_USE_INDEV_MOUSE
    /*------------------
     * Mouse
     * -----------------*/

    /*Initialize your touchpad if you have*/
    mouse_init();

    /*Register a mouse input device*/
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = mouse_read;
    indev_mouse = lv_indev_drv_register(&indev_drv);

    /*Set cursor. For simplicity set a HOME symbol now.*/
    lv_obj_t * mouse_cursor = lv_img_create(lv_scr_act());
    lv_img_set_src(mouse_cursor, LV_SYMBOL_HOME);
    lv_indev_set_cursor(indev_mouse, mouse_cursor);
#endif

#if (LV_USE_INDEV & LV_USE_INDEV_KEYPAD) == LV_USE_INDEV_KEYPAD
    /*------------------
     * Keypad
     * -----------------*/

    /*Initialize your keypad or keyboard if you have*/
    keypad_init();

    /*Register a keypad input device*/
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_KEYPAD;
    indev_drv.read_cb = keypad_read;
    indev_keypad = lv_indev_drv_register(&indev_drv);

    /*Later you should create group(s) with `lv_group_t * group = lv_group_create()`,
     *add objects to the group with `lv_group_add_obj(group, obj)`
     *and assign this input device to group to navigate in it:
     *`lv_indev_set_group(indev_keypad, group);`*/
#endif

#if (LV_USE_INDEV & LV_USE_INDEV_ENCODER) == LV_USE_INDEV_ENCODER
    /*------------------
     * Encoder
     * -----------------*/

    /*Initialize your encoder if you have*/
    encoder_init();

    /*Register a encoder input device*/
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_ENCODER;
    indev_drv.read_cb = encoder_read;
    indev_encoder = lv_indev_drv_register(&indev_drv);

    /*Later you should create group(s) with `lv_group_t * group = lv_group_create()`,
     *add objects to the group with `lv_group_add_obj(group, obj)`
     *and assign this input device to group to navigate in it:
     *`lv_indev_set_group(indev_encoder, group);`*/
#endif

#if (LV_USE_INDEV & LV_USE_INDEV_BUTTUN) == LV_USE_INDEV_BUTTUN
    /*------------------
     * Button
     * -----------------*/

    /*Initialize your button if you have*/
    button_init();

    /*Register a button input device*/
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_BUTTON;
    indev_drv.read_cb = button_read;
    indev_button = lv_indev_drv_register(&indev_drv);

    /*Assign buttons to points on the screen*/
    static const lv_point_t btn_points[2] = {
            {10, 10},   /*Button 0 -> x:10; y:10*/
            {40, 100},  /*Button 1 -> x:40; y:100*/
    };
    lv_indev_set_button_points(indev_button, btn_points);
#endif
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
#if (LV_USE_INDEV & LV_USE_INDEV_TOUCHPAD) == LV_USE_INDEV_TOUCHPAD
/*------------------
 * Touchpad
 * -----------------*/

/*Initialize your touchpad*/
static void touchpad_init(void)
{
    touch_fd = PrivOpen(PRIV_TOUCH_DEV,O_RDWR);
    if(touch_fd >= 0) {
        printf("touch fd = %d\n",touch_fd);
    } else {
        printf("open %s touch fd = %d failed.\n",PRIV_TOUCH_DEV,touch_fd);
    }
        
    /*Your code comes here*/
}

// static struct TouchDataStandard touch_data;

/*Will be called by the library to read the touchpad*/
static void touchpad_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
    static lv_coord_t last_x = 0;
    static lv_coord_t last_y = 0;

    static struct TouchDataStandard touch_data;

    /*Save the pressed coordinates and the state*/
    if (touchpad_is_pressed(&touch_data)) {
        touchpad_get_xy(&touch_data, &last_x, &last_y);
        data->state = LV_INDEV_STATE_PR;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }

    /*Set the last pressed coordinates*/
    data->point.x = last_x;
    data->point.y = last_y;
}

/*Return true is the touchpad is pressed*/
// static bool touchpad_is_pressed(void)
// {
//     int ret;
//     /*Your code comes here*/
//     // memset(&touch_data, 0 ,sizeof(TouchDataParam));
//     memset(&touch_data, 0 ,sizeof(struct TouchDataStandard));
//     ret = PrivRead(touch_fd, &touch_data, 1);
//     if (ret && touch_data.x >= 0 && touch_data.x < MY_INDEV_X
//         && touch_data.y >= 0 && touch_data.y < MY_INDEV_Y)
//     {
//         // printf("touch x %d touch y %d\n",touch_data.x,touch_data.y);
//         return true;
//     }
    
//     return false;
// }

uint32_t press_failed_cnt = 0;
static bool touchpad_is_pressed(struct TouchDataStandard* touch_data_ptr)
{
    int ret;
    /*Your code comes here*/
    memset(touch_data_ptr, 0 ,sizeof(struct TouchDataStandard));
    ret = PrivRead(touch_fd, touch_data_ptr, 1);
    if (ret && touch_data_ptr->x > 0 && touch_data_ptr->x < MY_INDEV_X
        && touch_data_ptr->y > 0 && touch_data_ptr->y < MY_INDEV_Y)
    {
        press_failed_cnt = 0;
        return true;
    }

    press_failed_cnt++;
    if (press_failed_cnt >= PRESS_FAILED_LIMIT) {
        PrivClose(touch_fd);
        touchpad_init();
        press_failed_cnt = 0;
    }
    
    return false;
}

/*Get the x and y coordinates if the touchpad is pressed*/
static void touchpad_get_xy(struct TouchDataStandard* touch_data_ptr,
    lv_coord_t* x, lv_coord_t* y) {
    /*Your code comes here*/

    (*x) = touch_data_ptr->x;
    (*y) = touch_data_ptr->y;
}

// static void touchpad_get_xy(lv_coord_t * x, lv_coord_t * y)
// {
//     /*Your code comes here*/

//     (*x) = touch_data.x;
//     (*y) = touch_data.y;
// }
#endif

#if (LV_USE_INDEV & LV_USE_INDEV_MOUSE) == LV_USE_INDEV_MOUSE
/*------------------
 * Mouse
 * -----------------*/

/*Initialize your mouse*/
static void mouse_init(void)
{
    /*Your code comes here*/
}

/*Will be called by the library to read the mouse*/
static void mouse_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
    /*Get the current x and y coordinates*/
    mouse_get_xy(&data->point.x, &data->point.y);

    /*Get whether the mouse button is pressed or released*/
    if(mouse_is_pressed()) {
        data->state = LV_INDEV_STATE_PR;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
}

/*Return true is the mouse button is pressed*/
static bool mouse_is_pressed(void)
{
    /*Your code comes here*/

    return false;
}

/*Get the x and y coordinates if the mouse is pressed*/
static void mouse_get_xy(lv_coord_t * x, lv_coord_t * y)
{
    /*Your code comes here*/

    (*x) = 0;
    (*y) = 0;
}
#endif



#if (LV_USE_INDEV & LV_USE_INDEV_KEYPAD) == LV_USE_INDEV_KEYPAD
/*------------------
 * Keypad
 * -----------------*/

/*Initialize your keypad*/
static void keypad_init(void)
{
    /*Your code comes here*/
}

/*Will be called by the library to read the mouse*/
static void keypad_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
    static uint32_t last_key = 0;

    /*Get the current x and y coordinates*/
    mouse_get_xy(&data->point.x, &data->point.y);

    /*Get whether the a key is pressed and save the pressed key*/
    uint32_t act_key = keypad_get_key();
    if(act_key != 0) {
        data->state = LV_INDEV_STATE_PR;

        /*Translate the keys to LVGL control characters according to your key definitions*/
        switch(act_key) {
        case 1:
            act_key = LV_KEY_NEXT;
            break;
        case 2:
            act_key = LV_KEY_PREV;
            break;
        case 3:
            act_key = LV_KEY_LEFT;
            break;
        case 4:
            act_key = LV_KEY_RIGHT;
            break;
        case 5:
            act_key = LV_KEY_ENTER;
            break;
        }

        last_key = act_key;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }

    data->key = last_key;
}

/*Get the currently being pressed key.  0 if no key is pressed*/
static uint32_t keypad_get_key(void)
{
    /*Your code comes here*/

    return 0;
}
#endif



#if (LV_USE_INDEV & LV_USE_INDEV_ENCODER) == LV_USE_INDEV_ENCODER
/*------------------
 * Encoder
 * -----------------*/

/*Initialize your keypad*/
static void encoder_init(void)
{
    /*Your code comes here*/
}

/*Will be called by the library to read the encoder*/
static void encoder_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{

    data->enc_diff = encoder_diff;
    data->state = encoder_state;
}

/*Call this function in an interrupt to process encoder events (turn, press)*/
static void encoder_handler(void)
{
    /*Your code comes here*/

    encoder_diff += 0;
    encoder_state = LV_INDEV_STATE_REL;
}
#endif

#if (LV_USE_INDEV & LV_USE_INDEV_BUTTUN) == LV_USE_INDEV_BUTTUN
/*------------------
 * Button
 * -----------------*/

/*Initialize your buttons*/
static void button_init(void)
{
    /*Your code comes here*/
}

/*Will be called by the library to read the button*/
static void button_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{

    static uint8_t last_btn = 0;

    /*Get the pressed button's ID*/
    int8_t btn_act = button_get_pressed_id();

    if(btn_act >= 0) {
        data->state = LV_INDEV_STATE_PR;
        last_btn = btn_act;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }

    /*Save the last pressed button's ID*/
    data->btn_id = last_btn;
}

/*Get ID  (0, 1, 2 ..) of the pressed button*/
static int8_t button_get_pressed_id(void)
{
    uint8_t i;

    /*Check to buttons see which is being pressed (assume there are 2 buttons)*/
    for(i = 0; i < 2; i++) {
        /*Return the pressed button's ID*/
        if(button_is_pressed(i)) {
            return i;
        }
    }

    /*No button pressed*/
    return -1;
}

/*Test if `id` button is pressed or not*/
static bool button_is_pressed(uint8_t id)
{

    /*Your code comes here*/

    return false;
}
#endif
#else /*Enable this file at the top*/

/*This dummy typedef exists purely to silence -Wpedantic.*/
typedef int keep_pedantic_happy;
#endif
