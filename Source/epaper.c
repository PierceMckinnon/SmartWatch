#include "EPD_1in54_V2.h"
#include "epaper.h"
#include "nrf_delay.h"
#include "nrf_drv_gpiote.h"
#include "timerconfig.h"

typedef struct EpaperPartialCluster {
  EpaperEntryCount_e epaperEntryCount;
  const unsigned char* epaperBaseBitmap;
} EpaperPartialCluster;

// static void epaperDisableDisplayChangeSources(void);
// static EpaperOptions_e epaperRefreshFromEntry(void);
static EpaperState_e epaperStateFromHomescreen(
    HomescreenStates_e homescreenState);

static uint8_t ImageBuffer[5000];
static EpaperState_e epaperState = epaperHomeScreen;
static EpaperBlock_e epaperBlockState = epaperNoBlock;
static EpaperMode_e epaperAwake = epaperSleepMode;
static void epaperDisplay(EpaperOptions_e refreshType,
                          EpaperSetupType_e epaperSetupType,
                          EpaperPartialCluster* const cluster);
static void epaperDisplayPartial(void);

static void epaperDisplayFilesHomescreen(
    const FilesHomescreenStates_e homescreenState,
    EpaperPrintPosition const* const printPosition);
static void epaperDisplayFilesUpload(
    const FilesUploadingStates_e uploadState,
    EpaperPrintPosition const* const printPosition);
static void epaperDisplayFilesDirectory(
    const FilesDirectoryStates_e directoryState,
    EpaperPrintPosition const* const printPosition);
static void epaperDisplayFilesDirectoryFilenames(void);
static void epaperDisplayFilesDirectoryTextFile(void);
static char const* const epaperDisplayTextFile(char const* const textFile,
                                               uint32_t fileLength);

typedef void (*epaperInitDisplay)(void);

static const epaperInitDisplay epaperInitDisplayFunctions[epaperStatesSize] = {
    homeScreenDisplay, calDisplay, emptyFunction,
    filesDisplay};  // should prob not be here
static EpaperEntryCount_e epaperEntryTracker[epaperStatesSize] = {
    epaperFirstEntry, epaperFirstEntry, epaperFirstEntry, epaperFirstEntry};

void epaperInit(void) {
  Paint_NewImage(ImageBuffer, EPD_1IN54_V2_WIDTH, EPD_1IN54_V2_HEIGHT, 0,
                 WHITE);
  Paint_SelectImage(ImageBuffer);
  Paint_Clear(WHITE);
  epaperTurnOnDisplay(epaperBitmap);
  EPD_1IN54_V2_Clear();
  nrf_delay_ms(500);
}

void epaperTesting(void) {
  printf("EPD_1in54_V2_test Demo\r\n");
  // DEV_Module_Init();
  epaperSleep();
  printf("e-Paper Init and Clear...\r\n");
  EPD_1IN54_V2_BMP_Init();

  EPD_1IN54_V2_Clear();
  nrf_delay_ms(4000);
  // while (1)
  //   ;

  // Create a new image cache

  /* you have to edit the startup_stm32fxxx.s file and set a big enough heap
   * size */

  Paint_NewImage(ImageBuffer, EPD_1IN54_V2_WIDTH, EPD_1IN54_V2_HEIGHT, 180,
                 WHITE);
  Paint_SelectImage(ImageBuffer);
  Paint_Clear(WHITE);

  Paint_DrawBitMap(MainMenu);
  EPD_1IN54_V2_Display(ImageBuffer);
  nrf_delay_ms(4000);

  // #if 1   //show image for array

  //   EPD_1IN54_V2_Display(BlackImage);
  //   nrf_delay_ms(5000);
  // #endif

  // #if 1   // Drawing on the image
  //     printf("Drawing\r\n");
  //     //1.Select Image
  //     Paint_SelectImage(BlackImage);
  //     Paint_Clear(WHITE);

  //     // 2.Drawing on the image
  //     Paint_DrawPoint(5, 10, BLACK, DOT_PIXEL_1X1, DOT_STYLE_DFT);
  //     Paint_DrawPoint(5, 25, BLACK, DOT_PIXEL_2X2, DOT_STYLE_DFT);
  //     Paint_DrawPoint(5, 40, BLACK, DOT_PIXEL_3X3, DOT_STYLE_DFT);
  //     Paint_DrawPoint(5, 55, BLACK, DOT_PIXEL_4X4, DOT_STYLE_DFT);

  //     Paint_DrawLine(20, 10, 70, 60, BLACK, DOT_PIXEL_1X1,
  //     LINE_STYLE_SOLID); Paint_DrawLine(70, 10, 20, 60, BLACK,
  //     DOT_PIXEL_1X1, LINE_STYLE_SOLID); Paint_DrawLine(170, 15, 170, 55,
  //     BLACK, DOT_PIXEL_1X1, LINE_STYLE_DOTTED); Paint_DrawLine(150, 35,
  //     190, 35, BLACK, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);

  //   Paint_DrawRectangle(20, 10, 70, 60, BLACK, DOT_PIXEL_1X1,
  //   DRAW_FILL_EMPTY); Paint_DrawRectangle(85, 10, 130, 60, BLACK,
  //   DOT_PIXEL_1X1, DRAW_FILL_FULL);

  //   Paint_DrawCircle(170, 35, 20, BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
  //   Paint_DrawCircle(170, 85, 20, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
  // Paint_DrawString_EN(5, 85, "waveshare", &Font20, BLACK, WHITE);
  // Paint_DrawNum(5, 110, 123456789, &Font20, BLACK, WHITE);

  // Paint_DrawString_CN(5, 135,"���abc", &Font12CN, BLACK, WHITE);
  // Paint_DrawString_CN(5, 155, "΢ѩ����", &Font24CN, WHITE, BLACK);

  //   EPD_1IN54_V2_Display(BlackImage);
  //   nrf_delay_ms(5000);

  for (int i = 0; i < 3; i++) {
    int movAmount = 180 - (40 * i);
    EPD_1IN54_V2_Init_Partial();
    Paint_DrawBitMap(MainMenu);
    Paint_DrawRectangle(movAmount, 55, movAmount - 45, 155, BLACK,
                        DOT_PIXEL_2X2, DRAW_FILL_EMPTY);
    // paintMe.DrawRectangle(0, movAmount, 19, movAmount + 20, BLACK);

    EPD_1IN54_V2_DisplayPart(ImageBuffer);
    nrf_delay_ms(2000);
    Paint_Clear(WHITE);
  }

  // return 0;
}

void epaperSleep(void) {
  EPD_1IN54_V2_BMP_Init();
  EPD_1IN54_V2_Clear();
  EPD_1IN54_V2_Sleep();
  DEV_Module_Exit();
  epaperAwake = epaperSleepMode;
}

static void epaperDisplayPartial(void) {
  EPD_1IN54_V2_Init_Partial();
  EPD_1IN54_V2_DisplayPart(ImageBuffer);
  timerEnableEpaperRefresh(100);
}

void epaperDisplay(EpaperOptions_e refreshType,
                   EpaperSetupType_e epaperSetupType,
                   EpaperPartialCluster* const cluster) {
  // epaperDisableDisplayChangeSources();
  if (epaperAwake == epaperSleepMode)
    SWERROR_HANDLER();
  static uint32_t countPartialRefresh = 0;
  if (ImageBuffer != NULL) {
    nrf_drv_timer_clear(&timerEpaperSleep);
    if (refreshType == epaperFullRefresh) {
      epaperTurnOnDisplay(epaperSetupType);
      EPD_1IN54_V2_Display(ImageBuffer);
      // timerEnableEpaperRefresh(1);
      countPartialRefresh = 0;
    } else if ((cluster->epaperEntryCount == epaperFirstEntry) ||
               countPartialRefresh > EPAPERMAXPARTIALREFRESH) {
      epaperTurnOnDisplay(epaperSetupType);
      EPD_1IN54_V2_Display(cluster->epaperBaseBitmap);
      nrf_delay_ms(1000);
      epaperDisplayPartial();
      countPartialRefresh = 0;
    } else {
      epaperDisplayPartial();
      countPartialRefresh++;
    }
  }
}

void epaperTurnOnDisplay(EpaperSetupType_e setupType) {
  DEV_Module_Init();
  if (setupType == epaperBitmap)
    EPD_1IN54_V2_BMP_Init();
  else
    EPD_1IN54_V2_Text_Init();
  epaperAwake = epaperAliveMode;
}

EpaperState_e epaperGetState(void) {
  return epaperState;
}

void epaperUpdateDateTime(CalDateTime const* const dateTime,
                          CalVariables_e highlight,
                          const EpaperPrintPosition printPosition) {
  epaperState = epaperDateTime;

  Paint_Clear(WHITE);
  Paint_SetRotate(270);
  Paint_DrawBitMap(Border);
  Paint_DrawDate(10, 40, dateTime, &Font24Wide, WHITE, BLACK);
  Paint_DrawTime(20, 120, dateTime, &Font40, WHITE, BLACK);

  if (highlight != calClear) {
    uint32_t recWidth;
    uint32_t recHeight;
    if (highlight == calDay || highlight == calMonth || highlight == calYear) {
      recWidth = DATERECWIDTH;
      recHeight = DATERECHEIGHT;
    } else {
      recWidth = TIMERECWIDTH;
      recHeight = TIMERECHEIGHT;
    }
    Paint_DrawRectangle(printPosition.x, printPosition.y,
                        printPosition.x + recWidth, printPosition.y + recHeight,
                        BLACK, DOT_PIXEL_2X2, DRAW_FILL_EMPTY);
  }
  EpaperPartialCluster cluster = {epaperEntryTracker[epaperDateTime], Border};
  epaperDisplay(epaperFullRefresh, epaperText, &cluster);
  epaperEntryTracker[epaperDateTime] = epaperMultiEntry;
}

void epaperExitToHome(void) {
  epaperEntryTracker[epaperHomeScreen] = epaperFirstEntry;
  homeScreenDisplay();
}

void epaperDisplayHomescreen(const EpaperPrintPosition printPosition) {
  epaperState = epaperHomeScreen;
  Paint_Clear(WHITE);
  Paint_SetRotate(0);
  Paint_DrawBitMap(MainMenu);

  Paint_DrawRectangle(printPosition.y, printPosition.x,
                      printPosition.y + HOMESCREENRECHEIGHT,
                      printPosition.x + HOMSESCREENRECWIDTH, BLACK,
                      DOT_PIXEL_2X2, DRAW_FILL_EMPTY);
  epaperDisplay(epaperFullRefresh, epaperBitmap, NULL);
  epaperEntryTracker[epaperHomeScreen] = epaperMultiEntry;
}

void epaperDisplayHomscreenSelect(HomescreenStates_e homescreenState) {
  EpaperState_e state = epaperStateFromHomescreen(homescreenState);
  epaperEntryTracker[state] = epaperFirstEntry;
  (*(epaperInitDisplayFunctions[state]))();
}

// static void epaperDisableDisplayChangeSources(void) {
//   epaperBlockState = epaperBlock;
//   buttonDisableInterrupts();
// }

void epaperEnableDisplayChangeSources(void) {
  epaperBlockState = epaperNoBlock;
  buttonEnableInterrupts();
}

EpaperBlock_e epaperGetBlockState(void) {
  return epaperBlockState;
}

// static EpaperOptions_e epaperRefreshFromEntry(void) {
//   return (epaperEntryTracker[epaperState] == epaperFirstEntry)
//              ? epaperFullRefresh
//              : epaperTryPartialRefresh;
// }

static EpaperState_e epaperStateFromHomescreen(
    HomescreenStates_e homescreenState) {
  EpaperState_e state;
  switch (homescreenState) {
    case (homescreenClock): {
      state = epaperDateTime;
      break;
    }
    case (homescreeenFiles): {
      state = epaperFileIO;
      break;
    }
    case (homescreenMusic): {
      state = epaperMusicIO;
      break;
    }
    default:
      SWERROR_HANDLER();
  }
  return state;
}

void epaperWakeFromSleep(void) {
  epaperEntryTracker[epaperState] = epaperFirstEntry;
  (*(epaperInitDisplayFunctions[epaperState]))();
}

EpaperMode_e epaperGetMode(void) {
  return epaperAwake;
}

static char const* const epaperDisplayTextFile(char const* const textFile,
                                               uint32_t fileLength) {
  char const* const newTextLocation =
      PaintDrawTextFile(textFile, fileLength, &Font16, WHITE, BLACK);

  epaperDisplay(epaperFullRefresh, epaperText, NULL);
  epaperEntryTracker[epaperHomeScreen] = epaperMultiEntry;

  return newTextLocation;
}

void epaperDisplayError(uint32_t error) {
  Paint_Clear(WHITE);
  Paint_SetRotate(270);
  Paint_DrawBitMap(Border);
  Paint_DrawNum(50, 50, error, &Font20, WHITE, BLACK);
  epaperDisplay(epaperFullRefresh, epaperText, NULL);
  epaperEntryTracker[epaperHomeScreen] = epaperMultiEntry;
}

void epaperDisplayFiles(FilesStatesTracker const* const filesState,
                        EpaperPrintPosition const* const printPosition) {
  epaperState = epaperFileIO;
  switch (filesState->filesGlobalState) {
    case (filesGlobalHomescreen): {
      epaperDisplayFilesHomescreen(filesState->filesHomescreenState,
                                   printPosition);
      break;
    }
    case (filesGlobalDirectory): {
      epaperDisplayFilesDirectory(filesState->filesDirectoryState,
                                  printPosition);
      break;
    }
    case (filesGlobalUpload): {
      epaperDisplayFilesUpload(filesState->filesUploadingState, printPosition);
      break;
    }
    default:
      SWERROR_HANDLER();
  }
  epaperEntryTracker[epaperHomeScreen] = epaperMultiEntry;
}

static void epaperDisplayFilesHomescreen(
    const FilesHomescreenStates_e homescreenState,
    EpaperPrintPosition const* const printPosition) {
  Paint_Clear(WHITE);
  Paint_SetRotate(0);
  Paint_DrawBitMap(FilesHomescreen);

  Paint_DrawRectangle(
      printPosition->y, printPosition->x, printPosition->y + FILESRECHEIGHT,
      printPosition->x + FILESRECWIDTH, BLACK, DOT_PIXEL_2X2, DRAW_FILL_EMPTY);
  epaperDisplay(epaperFullRefresh, epaperBitmap, NULL);
}

const char* epaperUploadStrings[filesUploadingStatesSize] = {
    "Uploading...", "Finished Uploading. Send Another Or Exit.  ",
    "Timeout Nothing Sent..."};
static void epaperDisplayFilesUpload(
    const FilesUploadingStates_e uploadState,
    EpaperPrintPosition const* const printPosition) {
  Paint_Clear(WHITE);
  Paint_SetRotate(270);
  Paint_DrawBitMap(Border);
  const char* uploadString = epaperUploadStrings[uploadState];
  Paint_DrawString_EN(printPosition->x, printPosition->y, uploadString, &Font20,
                      WHITE, BLACK);
  epaperDisplay(epaperFullRefresh, epaperText, NULL);
}

static void epaperDisplayFilesDirectory(
    const FilesDirectoryStates_e directoryState,
    EpaperPrintPosition const* const printPosition) {
  switch (directoryState) {
    case (filesDirectoryFileNames): {
      epaperDisplayFilesDirectoryFilenames();
      break;
    }
    case (filesDirectoryText): {
      epaperDisplayFilesDirectoryTextFile();
      break;
    }
    default:
      SWERROR_HANDLER();
  }
}

static void epaperDisplayFilesDirectoryFilenames(void) {
  Paint_Clear(WHITE);
  Paint_SetRotate(270);
  Paint_DrawBitMap(Border);

  for (uint32_t i = 0; i < DIRNUMTITLES; i++) {
    char const* const fileName = filesGetDirectoryFilenames(i);
    if (fileName != NULL) {
      DRAW_FILL fill = DRAW_FILL_EMPTY;

      if (filesGetDirectoryFilenamesHighlight() == i)
        fill = DRAW_FILL_FULL;
      Paint_DrawRectangle(DIRSELECTRECX, DIRTITLEY + i * DIRYOFFSET,
                          DIRSELECTRECX + DIRSELECRECWIDTH,
                          (DIRTITLEY + i * DIRYOFFSET) + DIRSELECRECHEIGHT,
                          BLACK, DOT_PIXEL_1X1, fill);

      Paint_DrawString_EN(DIRTITLEX, DIRTITLEY + i * DIRYOFFSET, fileName,
                          &Font16, WHITE, BLACK);
    }
  }

  epaperDisplay(epaperFullRefresh, epaperText, NULL);
}

static char const* newTextFileLocation;
void epaperDisplayFilesDirectoryTextFile(void) {
  Paint_Clear(WHITE);
  Paint_SetRotate(270);
  Paint_DrawBitMap(Border);

  FilesTextFilePackage textFilePackage = filesGetTextFile();
  newTextFileLocation =
      epaperDisplayTextFile(textFilePackage.textFile, textFilePackage.size);
  // save pointer to be retrieved
}

char const* const epaperGetNewTextFileLocation(void) {
  return newTextFileLocation;
}

void epaperDisplayTestingString(char const* testingString) {
  Paint_Clear(WHITE);
  Paint_SetRotate(270);
  Paint_DrawBitMap(Border);
  Paint_DrawString_EN(10, 10, testingString, &Font20, WHITE, BLACK);
  epaperDisplay(epaperFullRefresh, epaperText, NULL);
  epaperEntryTracker[epaperHomeScreen] = epaperMultiEntry;
}
