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
#include "esp_log.h"
#include "onewire_bus.h"

#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_ili9341.h"
#include "ds18b20.h"

extern "C" {
#include "lvgl.h"
}

static const char *TAG = "ds18b20";

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

TemperatureSensorData temperature_data;


[[noreturn]] void task_update_display(void *pvParameter) {

    Display_Il9341 display(PIN_NUM_MOSI, PIN_NUM_CLK, PIN_NUM_CS, PIN_NUM_DC, PIN_NUM_RST, PIN_NUM_BCKL);
    display.init();

    LVLG lvgl(display);
    lvgl.init();

    std::array<lv_obj_t *, 5> labels{};

    lv_obj_t * frame = lv_obj_create(lv_scr_act());
    lv_obj_set_size(frame, Display_Il9341::WIDTH - 10, Display_Il9341::HEIGHT - 10);
    lv_obj_center(frame);
    lv_obj_set_style_bg_color(frame, rgb_fix(DARK_BG), 0);
    lv_obj_set_style_border_color(frame, rgb_fix(WHITE), 0);
    lv_obj_set_style_border_width(frame, 2, 0);

    lv_obj_set_flex_flow(frame, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(frame, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(frame, 20, 0);

    lv_obj_t * header = lv_label_create(frame);
    lv_label_set_text(header, LV_SYMBOL_HOME " Temperature reading");
    lv_obj_set_style_text_color(header, rgb_fix(WHITE), 0);
    lv_obj_set_style_text_font(header, &lv_font_montserrat_16, 0);
    lv_obj_set_style_pad_bottom(header, 10, 0);
    lv_obj_set_size(header, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_border_color(header, rgb_fix(WHITE), 0);
    lv_obj_set_style_border_side(header, LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_set_style_border_width(header, 1, 0);

    for (size_t i = 0; i < 5; i++) {
        labels[i] = lv_label_create(frame);
        lv_label_set_text(labels[i], "Label Text");
        lv_obj_set_style_text_color(labels[i], rgb_fix(WHITE), 0);
        lv_obj_set_style_text_font(labels[i], &lv_font_montserrat_30, 0);
    }

    while (true) {
            auto readings = temperature_data.get_readings();
            for (size_t i = 0; i < 5; i++) {
                char buf[32];
                std::snprintf(buf, sizeof(buf), "Pipe %u: %.2f °C", static_cast<unsigned>(i + 1), readings[i]);
                lv_label_set_text(labels[i], buf);
            }

            lv_tick_inc(10);
            lv_timer_handler();
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
}

class DS18B20_Sensor {
public:
    explicit DS18B20_Sensor(const gpio_num_t pin) : pin(pin) {}
    void init() {
        init_onewire_bus();

        onewire_device_iter_handle_t iter = NULL;
        onewire_device_t next_onewire_device;
        esp_err_t search_result = ESP_OK;

        ESP_ERROR_CHECK(onewire_new_device_iter(bus, &iter));
        ESP_LOGI(TAG, "Device iterator created, start searching...");
        do {
            search_result = onewire_device_iter_get_next(iter, &next_onewire_device);
            if (search_result == ESP_OK) { // found a new device, let's check if we can upgrade it to a DS18B20
                ds18b20_config_t ds_cfg = {};
                onewire_device_address_t address;

                if (ds18b20_new_device_from_enumeration(&next_onewire_device, &ds_cfg, &device) == ESP_OK) {
                    ds18b20_get_device_address(device, &address);
                    ESP_LOGI(TAG, "Found a DS18B20[%d], address: %016llX", 1, address);
                } else {
                    ESP_LOGI(TAG, "Found an unknown device, address: %016llX", next_onewire_device.address);
                }
            }
        } while (search_result != ESP_ERR_NOT_FOUND);

    }

    [[nodiscard]] float read_temperature() const {
        assert(device != nullptr && "Device not initialized");

        float temperature = 0.0f;

        ESP_ERROR_CHECK(ds18b20_trigger_temperature_conversion_for_all(bus));
            ESP_ERROR_CHECK(ds18b20_get_temperature(device, &temperature));
            ESP_LOGI(TAG, "temperature read from DS18B20: %.2fC", temperature);

        return temperature;
    }

private:
    onewire_bus_handle_t bus = nullptr;
    ds18b20_device_handle_t device = nullptr;
    const gpio_num_t pin;

    void init_onewire_bus() {
       const onewire_bus_config_t bus_config = {
            .bus_gpio_num = this->pin,
            .flags = {
                .en_pull_up = true, // enable the internal pull-up resistor in case the external device didn't have one
            }
        };
        constexpr onewire_bus_rmt_config_t rmt_config = {
            .max_rx_bytes = 10, // 1byte ROM command + 8byte ROM number + 1byte device command
        };

        ESP_ERROR_CHECK(onewire_new_bus_rmt(&bus_config, &rmt_config, &bus));

        // ESP_ERROR_CHECK(onewire_new_device_iter(bus, &iter));
    }
};

[[noreturn]] void task_read_temperature_sensors(void *pvParameter) {
    DS18B20_Sensor sensor(GPIO_NUM_16);
    sensor.init();


    while (true) {
        const float temp = sensor.read_temperature();

        std::array<float, 5> new_readings{};
        for (size_t i = 0; i < 5; i++) {
            new_readings[i] = temp + std::rand() % 100 / 1000.0f;
        }

        temperature_data.set_readings(new_readings);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

extern "C" void app_main(void) {
    xTaskCreate(task_update_display, "task_update_display", 8192, nullptr, tskIDLE_PRIORITY + 1, nullptr);
    xTaskCreate(task_read_temperature_sensors, "task_read_temperature_sensors", 4096, nullptr, tskIDLE_PRIORITY + 1, nullptr);

    vTaskDelete(nullptr);
}
