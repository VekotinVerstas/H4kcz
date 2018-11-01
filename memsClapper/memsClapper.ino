#include <ESP8266WiFi.h>

/* Includes -------------------------------------------------------------------*/
extern "C" {
#include "user_interface.h"
#include "i2s_reg.h"
#include "slc_register.h"
#include "esp8266_peri.h"
  //#include <ArduinoSound.h>

  void rom_i2c_writeReg_Mask(int, int, int, int, int, int);
}

//#define DEBUG
#define PLOT

#define I2S_CLK_FREQ    160000000
#define I2S_24BIT       3     // I2S 24 bit half data
#define I2S_LEFT        2     // I2S RX Left channel

#define I2SI_DATA       12    // I2S data on GPIO12   // Wemos D6 - SPH DOUT
#define I2SI_BCK        13    // I2S clk on GPIO13    // Wemos D7 - SPH BCLK
#define I2SI_WS         14    // I2S select on GPIO14 // Wemos D5 - SPH LRCL

#define SLC_BUF_CNT     4     // Number of buffers in the I2S circular buffer
#define SLC_BUF_LEN     128   // Length of one buffer, in 32-bit words.

typedef struct {
  uint32_t blocksize: 12;
  uint32_t datalen: 12;
  uint32_t unused: 5;
  uint32_t sub_sof: 1;
  uint32_t eof: 1;
  uint32_t owner: 1;

  uint32_t buf_ptr;
  uint32_t next_link_ptr;
} sdio_queue_t;

static sdio_queue_t i2s_slc_items[SLC_BUF_CNT];  // I2S DMA buffer descriptors
static uint32_t *i2s_slc_buf_pntr[SLC_BUF_CNT];  // Pointer to the I2S DMA buffer data
static volatile uint32_t rx_buf_cnt = 0;
static volatile uint32_t rx_buf_idx = 0;
static volatile bool rx_buf_flag = false;

static volatile int32_t maxval = 0;
static volatile int32_t minval = 0;

int32_t convert(int32_t value);
void i2s_init();
void slc_init();
void i2s_set_rate(uint32_t rate);
void slc_isr(void *para);

int taputuskpl = 0;
int taputusDelay = 0;
int lamppupaalla = 0;

/* Main -----------------------------------------------------------------------*/
void setup()
{
  rx_buf_cnt = 0;

  pinMode(I2SI_WS, OUTPUT);
  pinMode(I2SI_BCK, OUTPUT);
  pinMode(I2SI_DATA, INPUT);
  pinMode(D1, OUTPUT);

  WiFi.forceSleepBegin();
  delay(500);

  Serial.begin(115200);
#ifdef DEBUG
  Serial.println("I2S receiver");
#endif

  slc_init();
#ifdef DEBUG
  Serial.println("SLC started");
#endif

  i2s_init();
#ifdef DEBUG
  Serial.println("Initialised");
#endif
delay(1000);
}

void loop()
{
  int32_t value;

#ifdef DEBUG
  delay(1000);
  Serial.print("rx_buf_cnt: ");
  Serial.println(rx_buf_cnt);
  rx_buf_cnt = 0;
#endif
#ifdef PLOT
  if (rx_buf_flag) {
    for (int x = 0; x < SLC_BUF_LEN; x++) {
      if (i2s_slc_buf_pntr[rx_buf_idx][x] > 0) {
        value = convert((int32_t)i2s_slc_buf_pntr[rx_buf_idx][x]);
        //value = (uint32_t)i2s_slc_buf_pntr[rx_buf_idx][x];
        if ( value > 10000 ) {
          if ( taputusDelay == 0 ) {
            taputusDelay = millis();
            taputuskpl = 1;
            Serial.print("Taputus: ");
            Serial.println(taputuskpl);
          }
          else if ((taputusDelay + 1000) > millis() ) {
            taputuskpl++;
            Serial.print("Taputus: ");
            Serial.println(taputuskpl);
            if ( taputuskpl >= 2 ) {
              taputuskpl = 0;
              taputusDelay = 0;
              if ( lamppupaalla ) {
                Serial.println("Lamppu off");
                lamppupaalla = 0;
                digitalWrite( D1, HIGH );
              }
              else {
                lamppupaalla = 1;
                Serial.println("Lamppu on");
                digitalWrite( D1, LOW );
             }
            }
          }
          else taputusDelay=0;
          delay(100);
        }
        //Serial.println(value);
      }
    }
    rx_buf_flag = false;
  }
  //Serial.println("");

#endif
}

/* Function definitions -------------------------------------------------------*/

/**
   Convert I2S data.
   Data is 18 bit signed, MSBit first, two's complement.
   Note: We can only send 31 cycles from ESP8266 so we only
   shift by 13 instead of 14.
*/
int32_t convert(int32_t value)
{
  //uint32_t sign;
  uint32_t mask;

  mask = (1 << 18);
  value >>= 13; //13
  if (value & mask) {
    value -= mask;
  }
  return value - 240200;
}

/**
   Initialise I2S as a RX master.
*/
void i2s_init()
{
  // Config RX pin function
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_I2SI_DATA);
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, FUNC_I2SI_BCK);
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, FUNC_I2SI_WS);

  // Enable a 160MHz clock
  I2S_CLK_ENABLE();

  // Reset I2S
  I2SC &= ~(I2SRST);
  I2SC |= I2SRST;
  I2SC &= ~(I2SRST);

  // Reset DMA
  I2SFC &= ~(I2SDE | (I2SRXFMM << I2SRXFM));

  // Enable DMA
  I2SFC |= I2SDE | (I2S_24BIT << I2SRXFM);

  // Set RX single channel (left)
  I2SCC &= ~((I2STXCMM << I2STXCM) | (I2SRXCMM << I2SRXCM));
  I2SCC |= (I2S_LEFT << I2SRXCM);
  i2s_set_rate(32000); //32000

  // Set RX data to be received
  I2SRXEN = SLC_BUF_LEN;

  // Bits mode
  I2SC |= (15 << I2SBM);

  // Start receiver
  I2SC |= I2SRXS;
}

/**
   Set I2S clock.
   I2S bits mode only has space for 15 extra bits,
   31 in total. The
*/
void i2s_set_rate(uint32_t rate)
{
  uint32_t i2s_clock_div = (I2S_CLK_FREQ / (rate * 31 * 2)) & I2SCDM;
  uint32_t i2s_bck_div = (I2S_CLK_FREQ / (rate * i2s_clock_div * 31 * 2)) & I2SBDM;
#ifdef DEBUG
  Serial.printf("Rate %u Div %u Bck %u Freq %u\n",
                rate, i2s_clock_div, i2s_bck_div, I2S_CLK_FREQ / (i2s_clock_div * i2s_bck_div * 31 * 2)) ;
#endif

  // RX master mode, RX MSB shift, right first, msb right
  I2SC &= ~(I2STSM | I2SRSM | (I2SBMM << I2SBM) | (I2SBDM << I2SBD) | (I2SCDM << I2SCD));
  I2SC |= I2SRF | I2SMR | I2SRMS | (i2s_bck_div << I2SBD) | (i2s_clock_div << I2SCD);
}

/**
   Initialize the SLC module for DMA operation.
   Counter intuitively, we use the TXLINK here to
   receive data.
*/
void slc_init()
{
  for (int x = 0; x < SLC_BUF_CNT; x++) {
    i2s_slc_buf_pntr[x] = (uint32_t *)malloc(SLC_BUF_LEN * 4);
    for (int y = 0; y < SLC_BUF_LEN; y++) i2s_slc_buf_pntr[x][y] = 0;

    i2s_slc_items[x].unused = 0;
    i2s_slc_items[x].owner = 1;
    i2s_slc_items[x].eof = 0;
    i2s_slc_items[x].sub_sof = 0;
    i2s_slc_items[x].datalen = SLC_BUF_LEN * 4;
    i2s_slc_items[x].blocksize = SLC_BUF_LEN * 4;
    i2s_slc_items[x].buf_ptr = (uint32_t)&i2s_slc_buf_pntr[x][0];
    i2s_slc_items[x].next_link_ptr = (int)((x < (SLC_BUF_CNT - 1)) ? (&i2s_slc_items[x + 1]) : (&i2s_slc_items[0]));
  }

  // Reset DMA
  ETS_SLC_INTR_DISABLE();
  SLCC0 |= SLCRXLR | SLCTXLR;
  SLCC0 &= ~(SLCRXLR | SLCTXLR);
  SLCIC = 0xFFFFFFFF;

  // Configure DMA
  SLCC0 &= ~(SLCMM << SLCM);      // Clear DMA MODE
  SLCC0 |= (1 << SLCM);           // Set DMA MODE to 1
  SLCRXDC |= SLCBINR | SLCBTNR;   // Enable INFOR_NO_REPLACE and TOKEN_NO_REPLACE

  // Feed DMA the 1st buffer desc addr
  SLCTXL &= ~(SLCTXLAM << SLCTXLA);
  SLCTXL |= (uint32_t)&i2s_slc_items[0] << SLCTXLA;

  ETS_SLC_INTR_ATTACH(slc_isr, NULL);

  // Enable EOF interrupt
  SLCIE = SLCITXEOF;
  ETS_SLC_INTR_ENABLE();

  // Start transmission
  SLCTXL |= SLCTXLS;
}

/**
   Triggered when SLC has finished writing
   to one of the buffers.
*/
void slc_isr(void *para)
{
  uint32_t status;

  status = SLCIS;
  SLCIC = 0xFFFFFFFF;

  if (status == 0) {
    return;
  }

  if (status & SLCITXEOF) {
    // We have received a frame
    ETS_SLC_INTR_DISABLE();
    sdio_queue_t *finished = (sdio_queue_t*)SLCTXEDA;

    finished->eof = 0;
    finished->owner = 1;
    finished->datalen = 0;

    for (int i = 0; i < SLC_BUF_CNT; i++) {
      if (finished == &i2s_slc_items[i]) {
        rx_buf_idx = i;
      }
    }
    rx_buf_cnt++;
    rx_buf_flag = true;
    ETS_SLC_INTR_ENABLE();
  }
}
