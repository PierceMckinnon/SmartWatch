#ifndef _SPI_CONFIG_H_
#define _SPI_CONFIG_H_

#include "nrf_drv_spi.h"

// Note: CS should be handled within device driver file, should not be specified
// in spi driver
#define SPI_TRANSFER(spi, buffer, bufferSize) \
  nrf_drv_spi_transfer(spi, buffer, bufferSize, NULL, 0)
#define SPI_RECEIVE(spi, returnBuffer, returnBufferSize) \
  nrf_drv_spi_transfer(spi, NULL, 0, returnBuffer, returnBufferSize)
#define SPI_TRANSFER_RECEIVE(spi, buffer, bufferSize, returnBuffer, \
                             returnBufferSize)                      \
  nrf_drv_spi_transfer(spi, buffer, bufferSize, returnBuffer, returnBufferSize)

typedef enum SpiInstances_e {
  spiEpaper,
  spiFlash,
  spiAccelerometer,
  spiInstanceSize
} SpiInstances_e;

void spiInitAll(void);
nrf_drv_spi_t const* spiGetDrvConfig(SpiInstances_e instance);

#endif  //_SPI_CONFIG_H_