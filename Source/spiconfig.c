#include "pinconfig.h"
#include "spiconfig.h"

// only have one set of miso/mosi so we only use 1 shared instance
static const nrf_drv_spi_t spiInstance = NRF_DRV_SPI_INSTANCE(0);

static nrf_drv_spi_config_t spiConfig = NRF_DRV_SPI_DEFAULT_CONFIG;

// static void spiInit(SpiInstances_e instance) { //may need to init differently
// later
//   spiConfig.ss_pin = NRF_DRV_SPI_PIN_NOT_USED;
//   spiConfig.sck_pin = SPICLK;
//   spiConfig.mosi_pin = SPIMOSI;
//   spiConfig.miso_pin = SPIMISO;
//   spiConfig.frequency = NRF_SPI_FREQ_500K;
//   APP_ERROR_CHECK(
//       nrf_drv_spi_init(spiInstanceList[instance], &spiConfig, NULL, NULL));
// }

void spiInitAll(void) {
  spiConfig.ss_pin = NRF_DRV_SPI_PIN_NOT_USED;
  spiConfig.sck_pin = SPICLK;
  spiConfig.mosi_pin = SPIMOSI;
  spiConfig.miso_pin = SPIMISO;
  spiConfig.frequency = NRF_SPI_FREQ_1M;
  APP_ERROR_CHECK(nrf_drv_spi_init(&spiInstance, &spiConfig, NULL, NULL));
}

nrf_drv_spi_t const* spiGetDrvConfig(void) {
  return &spiInstance;
}