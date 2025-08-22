#include "esp_vfs_fat.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "driver/spi_master.h"
#include "jpeg_decoder.h"   // comes with esp_jpeg component

#define LCD_HOST     SPI2_HOST
#define PIN_NUM_MOSI 37 
#define PIN_NUM_CLK  36 
#define PIN_NUM_CS   35
#define PIN_NUM_DC   38
#define PIN_NUM_RST  39 

#define IMAGE_WIDTH  240
#define IMAGE_HEIGHT 135
#define JPG_BPP      2        // RGB565 = 2 bytes

//#define BYTE_SWAP(v) ((v >> 8) | (v << 8))

static uint16_t *fb = NULL;  // DMA‑capable line or frame buffer

esp_lcd_panel_handle_t panel;

void lcd_init(void) 
{
    spi_bus_config_t buscfg = {
        .sclk_io_num = PIN_NUM_CLK,
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = -1,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 240 * 135 * 2 + 8
    };
    spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO);

    esp_lcd_panel_io_handle_t io;
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = PIN_NUM_DC,
        .cs_gpio_num = PIN_NUM_CS,
        .pclk_hz = 20 * 1000 * 1000,
        .trans_queue_depth = 10,
        .on_color_trans_done = NULL,
        .user_ctx = NULL,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8
    };
    esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, &io);

    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = PIN_NUM_RST,
        .rgb_endian = LCD_RGB_ENDIAN_RGB,
        .bits_per_pixel = 16,
    };
    esp_lcd_new_panel_st7789(io, &panel_config, &panel);

    esp_lcd_panel_reset(panel);
    esp_lcd_panel_init(panel);
    esp_lcd_panel_set_gap(panel, 40, 53);
    esp_lcd_panel_invert_color(panel, true);
    esp_lcd_panel_mirror(panel, true, false);
    esp_lcd_panel_swap_xy(panel, true);
    esp_lcd_panel_disp_on_off(panel, true);

    fb = heap_caps_malloc(IMAGE_WIDTH * IMAGE_HEIGHT * JPG_BPP, MALLOC_CAP_DMA);
}

void display_jpeg_espjpeg(const char *path)
{
    /* ------------- read entire JPEG file into RAM ------------- */
    FILE *f = fopen(path, "rb");
    if (!f) { printf("Open %s failed", path); return; }

    fseek(f, 0, SEEK_END);
    size_t jpg_size = ftell(f);
    rewind(f);

    uint8_t *jpg_buf = malloc(jpg_size);
    fread(jpg_buf, 1, jpg_size, f);
    fclose(f);

    /* ------------- prepare decode configuration ------------- */
    esp_jpeg_image_cfg_t cfg = {
        .indata       = jpg_buf,
        .indata_size  = jpg_size,
        .outbuf       = (uint8_t *)fb,
        .outbuf_size  = IMAGE_WIDTH * IMAGE_HEIGHT * JPG_BPP,
        .out_format   = JPEG_IMAGE_FORMAT_RGB565,       // direct 16‑bit
        .out_scale    = JPEG_IMAGE_SCALE_0,             // no scaling
        .flags = {
            .swap_color_bytes = 1,                      // ST7789 needs byte‑swapped RGB565
        }
    };
    esp_jpeg_image_output_t out;   /* will receive w, h, etc. */

    /* ------------- decode (blocking, ~10–50 ms) ------------- */
    esp_err_t ret = esp_jpeg_decode(&cfg, &out);
    free(jpg_buf);
    if (ret != ESP_OK) {
        ESP_LOGE("JPG", "Decoder err %d", ret);
        return;
    }

    esp_lcd_panel_draw_bitmap(panel,
                              0, 0,
                              IMAGE_WIDTH, IMAGE_HEIGHT,
                              fb);
}