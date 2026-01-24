#include <cstdio>
#include <cassert>
#include <string>
#include <array>
#include <atomic>
#include <cstring>

/* ================= ESP-IDF HEADERS ================= */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_heap_caps.h"

#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_ili9341.h"

extern "C" {
#include "lvgl.h"
}

constexpr gpio_num_t PIN_NUM_MOSI = GPIO_NUM_23;
constexpr gpio_num_t PIN_NUM_CLK = GPIO_NUM_18;
constexpr gpio_num_t PIN_NUM_CS = GPIO_NUM_5;
constexpr gpio_num_t PIN_NUM_DC = GPIO_NUM_2;
constexpr gpio_num_t PIN_NUM_RST = GPIO_NUM_4;
constexpr gpio_num_t PIN_NUM_BCKL = GPIO_NUM_15;

const lv_color_t WHITE = lv_color_make(255, 255, 255);
const lv_color_t BLACK = lv_color_make(0, 0, 0);

const lv_color_t DARK_BG = lv_color_make(0, 0, 0);

static lv_color_t rgb_fix(const lv_color_t color) {
    lv_color_t result;

    result.red = color.blue;
    result.green = color.red;
    result.blue = color.green;

    return result;
}

static void lvgl_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
    const auto panel = static_cast<esp_lcd_panel_handle_t>(lv_display_get_user_data(disp));
    esp_lcd_panel_draw_bitmap(panel, area->x1, area->y1, area->x2 + 1, area->y2 + 1, px_map);
}

static bool notify_lvgl_flush_ready(esp_lcd_panel_io_handle_t, esp_lcd_panel_io_event_data_t *, void *user_ctx) {
    const auto disp = static_cast<lv_display_t *>(user_ctx);
    lv_display_flush_ready(disp);
    return false;
}

static void lvgl_tick_cb(void *) {
    lv_tick_inc(2);
}

class Display_Il9341 {
public:
    Display_Il9341(const gpio_num_t mosi,const gpio_num_t clk,const gpio_num_t cs,const gpio_num_t dc,const gpio_num_t rst,const gpio_num_t bckl)
        : mosi(mosi), clk(clk), cs(cs), dc(dc), rst(rst), bckl(bckl) {

    }

    void init() {
        this->init_backlight();
        this->init_spi_bus();
        this->init_io();
        this->init_panel();
    }

    [[nodiscard]] esp_lcd_panel_io_handle_t get_io_handle() const {
        return this->io_handle;
    }

    [[nodiscard]] esp_lcd_panel_handle_t get_panel() const {
        return this->panel;
    }

    static constexpr uint32_t WIDTH = 240;
    static constexpr uint32_t HEIGHT = 320;

private:
    static constexpr auto  LCD_HOST =SPI2_HOST;

    const gpio_num_t mosi;
    const gpio_num_t clk;
    const gpio_num_t cs;
    const gpio_num_t dc;
    const gpio_num_t rst;
    const gpio_num_t bckl;

    esp_lcd_panel_io_handle_t io_handle = nullptr;
    esp_lcd_panel_handle_t panel = nullptr;
    

    void init_spi_bus() {
        spi_bus_config_t config{};
        config.sclk_io_num = this->clk;
        config.mosi_io_num = this->mosi;
        config.miso_io_num = -1;
        config.max_transfer_sz = WIDTH * 40 * sizeof(lv_color_t) ;
        ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &config, SPI_DMA_CH_AUTO));
    }

    void init_backlight() {
        gpio_config_t bk_gpio{};
        bk_gpio.pin_bit_mask = 1ULL << this->bckl;
        bk_gpio.mode = GPIO_MODE_OUTPUT;
        gpio_config(&bk_gpio);
        gpio_set_level(this->bckl, true);
    }

    void init_io() {
        esp_lcd_panel_io_spi_config_t config{};
        config.dc_gpio_num = this->dc;
        config.cs_gpio_num = this->cs;
        config.pclk_hz = 20 * 1000 * 1000;
        config.lcd_cmd_bits = 8;
        config.lcd_param_bits = 8;
        config.spi_mode = 0;
        config.trans_queue_depth = 10;

        ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(LCD_HOST, &config, &this->io_handle));}

    void init_panel() {
        esp_lcd_panel_dev_config_t config{};
        config.reset_gpio_num = this->rst;
        config.rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR; // matches your module
        config.bits_per_pixel = 16;
        ESP_ERROR_CHECK(esp_lcd_new_panel_ili9341(io_handle, &config, &panel));

        esp_lcd_panel_reset(panel);
        esp_lcd_panel_init(panel);
        esp_lcd_panel_disp_on_off(panel, true);
        ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel, false, true));

    }
};

class LVLG {
public:
    LVLG(Display_Il9341 display) : io_handle(display.get_io_handle()), panel(display.get_panel()) {}

    void init() {
        lv_init();

        lv_display_t *disp = lv_display_create(Display_Il9341::WIDTH, Display_Il9341::HEIGHT);
        lv_display_set_flush_cb(disp, lvgl_flush_cb);
        lv_display_set_user_data(disp, panel);

        esp_lcd_panel_io_callbacks_t cbs{};
        cbs.on_color_trans_done = notify_lvgl_flush_ready;
        ESP_ERROR_CHECK(esp_lcd_panel_io_register_event_callbacks(io_handle, &cbs, disp));

        static lv_color_t buf1[Display_Il9341::HEIGHT * 40];
        static lv_color_t buf2[Display_Il9341::HEIGHT * 40];
        lv_display_set_buffers(disp, buf1, buf2, sizeof(buf1), LV_DISPLAY_RENDER_MODE_PARTIAL);

        lv_display_set_theme(disp, lv_theme_default_init(
            disp,
            DARK_BG,
            rgb_fix(lv_color_make(255,255,255)),
            true,
            &lv_font_montserrat_20
        ));
        lv_obj_set_style_bg_color(lv_scr_act(), DARK_BG, 0);
    }

private:
    esp_lcd_panel_io_handle_t io_handle;
    esp_lcd_panel_handle_t panel;
};

struct TemperatureSensorData {
    static constexpr uint32_t PIPE_1 = 0;
    static constexpr uint32_t PIPE_2 = 1;
    static constexpr uint32_t PIPE_3 = 2;
    static constexpr uint32_t PIPE_4 = 3;
    static constexpr uint32_t PIPE_5 = 4;

    void set_readings(const std::array<float , 5> &new_readings) {
        std::lock_guard lock(this->semaphore);
        std::memcpy(this->readings.data(), new_readings.data(), sizeof(float) * 5);
    }

    std::array<float, 5> get_readings() {
        std::lock_guard lock(this->semaphore);
        return this->readings;
    }

private:
    std::mutex semaphore;
    std::array<float, 5> readings{};
};

TemperatureSensorData temperature;

[[noreturn]] void task_update_display(void *pvParameter) {

    Display_Il9341 display(PIN_NUM_MOSI, PIN_NUM_CLK, PIN_NUM_CS, PIN_NUM_DC, PIN_NUM_RST, PIN_NUM_BCKL);
    display.init();

    LVLG lvgl(display);
    lvgl.init();

    std::array<lv_obj_t *, 5> labels{};

    for (size_t i = 0; i < 5; i++) {
        labels[i] = lv_label_create(lv_screen_active());
        lv_obj_align(labels[i], LV_ALIGN_TOP_LEFT, 10, 10 + i * 40);
        lv_obj_set_style_text_color(labels[i], rgb_fix(WHITE), 0);
        lv_obj_set_style_text_font(labels[i], &lv_font_montserrat_20, 0);
        // lv_obj_set_style_text_opa(labels[i], LV_OPA_COVER, 0);
        // lv_obj_set_style_text_color(labels[i], lv_color_black(), 0);
    }


    while (true) {
            auto readings = temperature.get_readings();
            for (size_t i = 0; i < 5; i++) {
                char buf[32];
                std::snprintf(buf, sizeof(buf), "Pipe %u: %.2f °C", static_cast<unsigned>(i + 1), readings[i]);
                lv_label_set_text(labels[i], buf);
            }

            lv_tick_inc(100);
            lv_timer_handler();
            vTaskDelay(pdMS_TO_TICKS(100));
        }
}

[[noreturn]] void task_read_temperature_sensors(void *pvParameter) {
    while (true) {
        std::array<float, 5> new_readings{};
        for (size_t i = 0; i < 5; i++) {
            new_readings[i] = 20.0f + static_cast<float>(std::rand() % 1000) / 50.0f; // Simulated temperature
        }
        temperature.set_readings(new_readings);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

extern "C" [[noreturn]] void app_main(void) {

    xTaskCreate(task_update_display, "task_update_display", 8192, nullptr, tskIDLE_PRIORITY + 1, nullptr);
    xTaskCreate(task_read_temperature_sensors, "task_read_temperature_sensors", 4096, nullptr, tskIDLE_PRIORITY + 1, nullptr);

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
