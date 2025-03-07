// Kafkar.com

#include <Arduino.h>
#include <SPI.h>

// include the installed LVGL- Light and Versatile Graphics Library - https://github.com/lvgl/lvgl
#include <lvgl.h>

// include the installed "TFT_eSPI" library by Bodmer to interface with the TFT Display - https://github.com/Bodmer/TFT_eSPI
#include <TFT_eSPI.h>

// include the installed the "XPT2046_Touchscreen" library by Paul Stoffregen to use the Touchscreen - https://github.com/PaulStoffregen/XPT2046_Touchscreen
#include <XPT2046_Touchscreen.h>


// Create a instance of the TFT_eSPI class
TFT_eSPI tft = TFT_eSPI();

// Set the pius of the xpt2046 touchscreen
#define XPT2046_IRQ 36  // T_IRQ
#define XPT2046_MOSI 32 // T_DIN
#define XPT2046_MISO 39 // T_OUT
#define XPT2046_CLK 25  // T_CLK
#define XPT2046_CS 33   // T_CS

// Create a instance of the SPIClass and XPT2046_Touchscreen classes
SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

// Define the size of the TFT display
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320

// Define the size of the buffer for the TFT display
#define DRAW_BUF_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 10 * (LV_COLOR_DEPTH / 8))

// Touchscreen coordinates: (x, y) and pressure (z)
int x, y, z;

// Create variables for the LVGL objects
lv_obj_t *led1;
lv_obj_t *led3;
lv_obj_t * btn_label;

// Create a variable to store the LED state
bool ledsOff = false;
bool rightLedOn = true;

// Create a buffer for drawing
uint32_t draw_buf[DRAW_BUF_SIZE / 4];


// Get the Touchscreen data
void touchscreen_read(lv_indev_t * indev, lv_indev_data_t * data) {
  // Checks if Touchscreen was touched, and prints X, Y and Pressure (Z)
  if(touchscreen.tirqTouched() && touchscreen.touched()) {
    // Get Touchscreen points
    TS_Point p = touchscreen.getPoint();
    // Calibrate Touchscreen points with map function to the correct width and height
    x = map(p.x, 200, 3700, 1, SCREEN_WIDTH);
    y = map(p.y, 240, 3800, 1, SCREEN_HEIGHT);
    z = p.z;

    data->state = LV_INDEV_STATE_PRESSED;

    // Set the coordinates
    data->point.x = x;
    data->point.y = y;
  }
  else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}


// Callback that is triggered when btn2 is clicked/toggled
static void event_handler_btn2(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t * obj = (lv_obj_t*) lv_event_get_target(e);
  if(code == LV_EVENT_VALUE_CHANGED) {
    LV_UNUSED(obj);
    if(ledsOff==true){
      if(lv_obj_has_state(obj, LV_STATE_CHECKED)==true) 
      {
        lv_led_off(led1);
        lv_led_on(led3);
        lv_label_set_text(btn_label, "Left");
        rightLedOn = true;
      }
      else
      {
        lv_led_on(led1);
        lv_led_off(led3);
        lv_label_set_text(btn_label, "Right");
        rightLedOn = false;
      }
      //LV_LOG_USER("Toggled %s", lv_obj_has_state(obj, LV_STATE_CHECKED) ? "on" : "off");
    }
  }
}

static void event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = (lv_obj_t*)lv_event_get_target(e);
    if(code == LV_EVENT_VALUE_CHANGED) {
        LV_UNUSED(obj);
        if(lv_obj_has_state(obj, LV_STATE_CHECKED)==true)
        {
          if(rightLedOn==true)
          {
            lv_led_off(led1);
            lv_led_on(led3);
          }
          else
          {
            lv_led_off(led3);
            lv_led_on(led1);
          }
          ledsOff = true;
        }
        else
        {
          lv_led_off(led1);
          lv_led_off(led3);
          ledsOff = false;
        }
        //LV_LOG_USER("State: %s\n", lv_obj_has_state(obj, LV_STATE_CHECKED) ? "On" : "Off");
    }
}


static lv_obj_t * slider_label;
// Callback that prints the current slider value on the TFT display and Serial Monitor for debugging purposes
static void slider_event_callback(lv_event_t * e) {
  lv_obj_t * slider = (lv_obj_t*) lv_event_get_target(e);
  char buf[8];
  lv_snprintf(buf, sizeof(buf), "%d%%", (int)lv_slider_get_value(slider));
  lv_label_set_text(slider_label, buf);
  lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
}


void lv_create_main_gui(void) {



  /*Create a LED and switch it OFF*/
  led1  = lv_led_create(lv_screen_active());
  lv_obj_align(led1, LV_ALIGN_CENTER, -100, 0);
  lv_led_set_brightness(led1, 50);
  lv_led_set_color(led1, lv_palette_main(LV_PALETTE_LIGHT_GREEN));
  lv_led_off(led1);

 
  /*Copy the previous LED and switch it ON*/
  led3  = lv_led_create(lv_screen_active());
  lv_obj_align(led3, LV_ALIGN_CENTER, 100, 0);
  lv_led_set_brightness(led3, 250);
  lv_led_set_color(led3, lv_palette_main(LV_PALETTE_LIGHT_GREEN));
  lv_led_on(led3);

  // Create a text label aligned center on top ("Hello, Kafkar.com!")
  lv_obj_t * text_label = lv_label_create(lv_screen_active());
  lv_label_set_long_mode(text_label, LV_LABEL_LONG_WRAP);    // Breaks the long lines
  lv_label_set_text(text_label, "Hello, Kafkar.com!");
  lv_obj_set_width(text_label, 150);    // Set smaller width to make the lines wrap
  lv_obj_set_style_text_align(text_label, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(text_label, LV_ALIGN_CENTER, 0, -90);


  lv_obj_t * sw;

  sw = lv_switch_create(lv_screen_active());
  lv_obj_add_event_cb(sw, event_handler, LV_EVENT_ALL, NULL);
  lv_obj_add_flag(sw, LV_OBJ_FLAG_EVENT_BUBBLE);
  lv_obj_align(sw, LV_ALIGN_CENTER, 0, -60);
  lv_obj_add_state(sw, LV_STATE_CHECKED);

  // Create a Toggle button (btn2)
  lv_obj_t *btn2 = lv_button_create(lv_screen_active());
  lv_obj_add_event_cb(btn2, event_handler_btn2, LV_EVENT_ALL, NULL);
  lv_obj_align(btn2, LV_ALIGN_CENTER, 0, 0);
  lv_obj_add_flag(btn2, LV_OBJ_FLAG_CHECKABLE);
  lv_obj_set_height(btn2, LV_SIZE_CONTENT);

  btn_label = lv_label_create(btn2);
  lv_label_set_text(btn_label, "Left");
  lv_obj_center(btn_label);
  
  // Create a slider aligned in the center bottom of the TFT display
  lv_obj_t * slider = lv_slider_create(lv_screen_active());
  lv_obj_align(slider, LV_ALIGN_CENTER, 0, 60);
  lv_obj_add_event_cb(slider, slider_event_callback, LV_EVENT_VALUE_CHANGED, NULL);
  lv_slider_set_range(slider, 0, 100);
  lv_obj_set_style_anim_duration(slider, 2000, 0);

  // Create a label below the slider to display the current slider value
  slider_label = lv_label_create(lv_screen_active());
  lv_label_set_text(slider_label, "0%");
  lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
}

void setup() {
  String LVGL_Arduino = String("LVGL Library Version: ") + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
  Serial.begin(115200);
  Serial.println(LVGL_Arduino);
  
  // Start LVGL
  lv_init();

  // Start the SPI for the touchscreen and init the touchscreen
  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  // Set the Touchscreen rotation in landscape mode
  // Note: in some displays, the touchscreen might be upside down, so you might need to set the rotation to 0: touchscreen.setRotation(0);
  touchscreen.setRotation(2);

  // Create a display object
  lv_display_t *disp;

  // Initialize the TFT display using the TFT_eSPI library
  disp = lv_tft_espi_create(SCREEN_WIDTH, SCREEN_HEIGHT, draw_buf, sizeof(draw_buf));
  lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_270);
    
  // Initialize an LVGL input device object (Touchscreen)
  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);

  // Set the callback function to read Touchscreen input
  lv_indev_set_read_cb(indev, touchscreen_read);

  // Function to draw the GUI (text, buttons and sliders)
  lv_create_main_gui();
}

void loop() {
  lv_task_handler();  // let the GUI do its work
  lv_tick_inc(5);     // tell LVGL how much time has passed
  delay(5);           // let this time pass
}