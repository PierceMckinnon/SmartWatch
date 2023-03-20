#include "pinconfig.h"
#include "spiconfig.h"

static const nrf_drv_spi_t spiEpaperInstance = NRF_DRV_SPI_INSTANCE(0);
static const nrf_drv_spi_t spiFlashInstance = NRF_DRV_SPI_INSTANCE(1);
static const nrf_drv_spi_t spiAccInstance = NRF_DRV_SPI_INSTANCE(2);

static nrf_drv_spi_t const* spiInstanceList[spiInstanceSize] = {
    &spiEpaperInstance, &spiFlashInstance, &spiAccInstance};
static nrf_drv_spi_config_t spiConfig = NRF_DRV_SPI_DEFAULT_CONFIG;

static void spiInit(SpiInstances_e instance) {
  spiConfig.ss_pin = NRF_DRV_SPI_PIN_NOT_USED;
  spiConfig.sck_pin = SPICLK;
  spiConfig.mosi_pin = SPIMOSI;
  spiConfig.miso_pin = SPIMISO;
  spiConfig.frequency = NRF_SPI_FREQ_500K;
  APP_ERROR_CHECK(
      nrf_drv_spi_init(spiInstanceList[instance], &spiConfig, NULL, NULL));
}

void spiInitAll(void) {
  for (uint32_t i = 0; i < spiInstanceSize; i++) {
    spiInit(i);
  }
}

nrf_drv_spi_t const* spiGetDrvConfig(SpiInstances_e instance) {
  return spiInstanceList[instance];
}