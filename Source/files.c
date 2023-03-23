#include "files.h"

#include <stdio.h>
#include "epaper.h"
#include "nrf_gpio.h"
#include "sendreceive.h"
#include "w25qxx.h"
// Homescreeen Presses
static void filesHomescreenHandleTopRightPress(void);
static void filesHomescreenHandleBottomRightPress(void);
static void filesHomescreenHandleBottomLeftPress(void);
// Upload Presses
static void filesUploadHandleBottomLeftPress(void);
// Directory Presses
static void filesDirectoryHandleTopLeftPress(void);
static void filesDirectoryHandleTopRightPress(void);
static void filesDirectoryHandleBottomRightPress(void);
static void filesDirectoryHandleBottomLeftPress(void);
// Directory Title State
static void filesDirectoryTitleTopLeftPress(void);
static void filesDirectoryTitleTopRightPress(void);
static void filesDirectoryTitleBottomLeftPress(void);
static void filesDirectoryTitleBottomRightPress(void);
// Directory text state
// static void filesDirectoryTextTopLeftPress(void);
static void filesDirectoryTextTopRightPress(void);
// static void filesDirectoryTextBottomRightPress(void);
static void filesDirectoryTextBottomLeftPress(void);
// static void filesDisplayDirectory(void);
static void filesDisplayErrorTesting(char const* text, int errorNum);

static void filesTextFileDisplay(void);

// Directory Filenames
static void filesDisplayDirectoryFilenames(
    FilesDirectoryHighlightDirection_e highlightDirection);
static void filesDisplayDirectoryFilenamesPageUp(void);
static void filesDisplayDirectoryFilenamesPageDown(void);
static FilesDirectoryHighlight_e filesHandleHighlightChange(
    FilesDirectoryHighlightDirection_e highlightDirection);

// Directory Text
static void filesDisplayDirectoryText(void);

static FilesGlobalStates_e filesResolveHomescreenSelect(void);

static void filesFSMount(void);

const ButtonHandlerSetup filesButtonHandlers = {
    .topLeftButtonPress = filesHandleTopLeftPress,
    .topRightButtonPress = filesHandleTopRightPress,
    .bottomLeftButtonPress = filesHandleBottomLeftPress,
    .bottomRightButtonPress = filesHandleBottomRightPress};

static const ButtonHandlerSetup filesHomescreenButtonHandlers = {
    .topLeftButtonPress = emptyFunction,
    .topRightButtonPress = filesHomescreenHandleTopRightPress,
    .bottomLeftButtonPress = filesHomescreenHandleBottomLeftPress,
    .bottomRightButtonPress = filesHomescreenHandleBottomRightPress};

static const ButtonHandlerSetup filesUploadButtonHandlers = {
    .topLeftButtonPress = emptyFunction,
    .topRightButtonPress = emptyFunction,
    .bottomLeftButtonPress = filesUploadHandleBottomLeftPress,
    .bottomRightButtonPress = emptyFunction};

static const ButtonHandlerSetup filesDirectoryButtonHandlers = {
    .topLeftButtonPress = filesDirectoryHandleTopLeftPress,
    .topRightButtonPress = filesDirectoryHandleTopRightPress,
    .bottomLeftButtonPress = filesDirectoryHandleBottomLeftPress,
    .bottomRightButtonPress = filesDirectoryHandleBottomRightPress};

static const FilesStatesTracker filesDefaultStates = {
    filesGlobalHomescreen, filesHomescreenDirectory, filesDirectoryFileNames,
    filesUploading};

static FilesStatesTracker filesStates = filesDefaultStates;

static ButtonHandlerSetup const* const
    filesButtonHandlersList[filesGlobalStatesSize] = {
        &filesHomescreenButtonHandlers, &filesDirectoryButtonHandlers,
        &filesUploadButtonHandlers};

const EpaperPrintPosition
    filesHomescreenDisplayLocations[filesHomescreenStateSize] = {
        {.x = UPLOADX, .y = UPLOADY},
        {.x = DIRECTORYX, .y = DIRECTORYY}};

const EpaperPrintPosition filesUartPrintPosition = {.x = UARTX, .y = UARTY};

static const char* mainFolderName = "/MainFolder";

// LITTLEFS
//  variables used by the filesystem
lfs_t lfs;
lfs_file_t file;

int filesReadWrapper(const struct lfs_config* c,
                     lfs_block_t block,
                     lfs_off_t off,
                     void* buffer,
                     lfs_size_t size) {
  uint32_t startAddress = block * (c->block_size) + off;

  W25qxx_ReadBytes((uint8_t*)buffer, startAddress, size);
  return 0;
}

int filesWriteWrapper(const struct lfs_config* c,
                      lfs_block_t block,
                      lfs_off_t off,
                      const void* buffer,
                      lfs_size_t size) {
  uint32_t startAddress = block * (c->block_size) + off;
  for (int i = 0; i < size; i++) {
    W25qxx_WriteByte(*((uint8_t*)buffer + i), startAddress + i);
  }
  return 0;
}

int filesEraseWrapper(const struct lfs_config* c, lfs_block_t block) {
  W25qxx_EraseBlock(block);
  return 0;
}

int filesSyncWrapper(const struct lfs_config* c) {
  // no caching, do nothing
  return 0;
}

const struct lfs_config lfsConfig = {
    // block device operations
    .read = filesReadWrapper,
    .prog = filesWriteWrapper,
    .erase = filesEraseWrapper,
    .sync = filesSyncWrapper,

    // block device configuration
    .read_size = 1,
    .prog_size = 1,
    .block_size = 65536,
    .block_count = 256,
    .block_cycles = 500,
    .cache_size = 64,
    .lookahead_size = 64};

void filesFSInit(void) {
  filesFSMount();
  lfs_mkdir(&lfs, mainFolderName);
}

static void filesFSMount(void) {
  int err = lfs_mount(&lfs, &lfsConfig);
  if (err) {
    int otherError = lfs_format(&lfs, &lfsConfig);
    if (otherError) {
      filesDisplayErrorTesting("still", otherError);
    }

    int stillError = lfs_mount(&lfs, &lfsConfig);
    if (stillError) {
      filesDisplayErrorTesting("mount", stillError);
    }
  }
}

char fileNameBuf[FILESMAXTITLESIZE * 2];
void filesFileOpenForWrite(char const* title) {
  snprintf(fileNameBuf, sizeof(fileNameBuf), "%s/%s", mainFolderName, title);
  int err = lfs_file_open(&lfs, &file, fileNameBuf, LFS_O_RDWR | LFS_O_CREAT);
  if (err) {
    filesDisplayErrorTesting("OPEN", err);
  }
}

void filesFileWrite(uint8_t* textFile, uint32_t sizeToWrite) {
  int err = lfs_file_write(&lfs, &file, (void const*)textFile, sizeToWrite);

  if (err <= 0) {
    filesDisplayErrorTesting("WRITE", err);
  }
}

void filesFileClose(void) {
  int err = lfs_file_close(&lfs, &file);
  if (err) {
    if (err <= 0) {
      filesDisplayErrorTesting("CLOSE", err);
    }
  }
}
// uint8_t rawData[687] = {
//     0x54, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20, 0x61, 0x20, 0x74,
//     0x65, 0x78, 0x74, 0x66, 0x69, 0x6C, 0x65, 0x20, 0x74, 0x65, 0x73,
//     0x74, 0x3A, 0x0D, 0x0A, 0x0D, 0x0A, 0x41, 0x20, 0x48, 0x61, 0x72,
//     0x65, 0x20, 0x77, 0x61, 0x73, 0x20, 0x6D, 0x61, 0x6B, 0x69, 0x6E,
//     0x67, 0x20, 0x66, 0x75, 0x6E, 0x20, 0x6F, 0x66, 0x20, 0x74, 0x68,
//     0x65, 0x20, 0x54, 0x6F, 0x72, 0x74, 0x6F, 0x69, 0x73, 0x65, 0x20,
//     0x6F, 0x6E, 0x65, 0x20, 0x64, 0x61, 0x79, 0x20, 0x66, 0x6F, 0x72,
//     0x20, 0x62, 0x65, 0x69, 0x6E, 0x67, 0x20, 0x73, 0x6F, 0x20, 0x73,
//     0x6C, 0x6F, 0x77, 0x2E, 0x0D, 0x0A, 0x22, 0x44, 0x6F, 0x20, 0x79,
//     0x6F, 0x75, 0x20, 0x65, 0x76, 0x65, 0x72, 0x20, 0x67, 0x65, 0x74,
//     0x20, 0x61, 0x6E, 0x79, 0x77, 0x68, 0x65, 0x72, 0x65, 0x3F, 0x22,
//     0x20, 0x68, 0x65, 0x20, 0x61, 0x73, 0x6B, 0x65, 0x64, 0x20, 0x77,
//     0x69, 0x74, 0x68, 0x20, 0x61, 0x20, 0x6D, 0x6F, 0x63, 0x6B, 0x69,
//     0x6E, 0x67, 0x20, 0x6C, 0x61, 0x75, 0x67, 0x68, 0x2E, 0x0D, 0x0A,
//     0x22, 0x59, 0x65, 0x73, 0x2C, 0x22, 0x20, 0x72, 0x65, 0x70, 0x6C,
//     0x69, 0x65, 0x64, 0x20, 0x74, 0x68, 0x65, 0x20, 0x54, 0x6F, 0x72,
//     0x74, 0x6F, 0x69, 0x73, 0x65, 0x2C, 0x20, 0x22, 0x61, 0x6E, 0x64,
//     0x20, 0x49, 0x20, 0x67, 0x65, 0x74, 0x20, 0x74, 0x68, 0x65, 0x72,
//     0x65, 0x20, 0x73, 0x6F, 0x6F, 0x6E, 0x65, 0x72, 0x20, 0x74, 0x68,
//     0x61, 0x6E, 0x20, 0x79, 0x6F, 0x75, 0x20, 0x74, 0x68, 0x69, 0x6E,
//     0x6B, 0x2E, 0x20, 0x49, 0x27, 0x6C, 0x6C, 0x20, 0x72, 0x75, 0x6E,
//     0x20, 0x79, 0x6F, 0x75, 0x20, 0x61, 0x20, 0x72, 0x61, 0x63, 0x65,
//     0x20, 0x61, 0x6E, 0x64, 0x20, 0x70, 0x72, 0x6F, 0x76, 0x65, 0x20,
//     0x69, 0x74, 0x2E, 0x22, 0x0D, 0x0A, 0x54, 0x68, 0x65, 0x20, 0x48,
//     0x61, 0x72, 0x65, 0x20, 0x77, 0x61, 0x73, 0x20, 0x6D, 0x75, 0x63,
//     0x68, 0x20, 0x61, 0x6D, 0x75, 0x73, 0x65, 0x64, 0x20, 0x61, 0x74,
//     0x20, 0x74, 0x68, 0x65, 0x20, 0x69, 0x64, 0x65, 0x61, 0x20, 0x6F,
//     0x66, 0x20, 0x72, 0x75, 0x6E, 0x6E, 0x69, 0x6E, 0x67, 0x20, 0x61,
//     0x20, 0x72, 0x61, 0x63, 0x65, 0x20, 0x77, 0x69, 0x74, 0x68, 0x20,
//     0x74, 0x68, 0x65, 0x20, 0x54, 0x6F, 0x72, 0x74, 0x6F, 0x69, 0x73,
//     0x65, 0x2C, 0x20, 0x62, 0x75, 0x74, 0x20, 0x66, 0x6F, 0x72, 0x20,
//     0x74, 0x68, 0x65, 0x20, 0x66, 0x75, 0x6E, 0x20, 0x6F, 0x66, 0x20,
//     0x74, 0x68, 0x65, 0x20, 0x74, 0x68, 0x69, 0x6E, 0x67, 0x20, 0x68,
//     0x65, 0x20, 0x61, 0x67, 0x72, 0x65, 0x65, 0x64, 0x2E, 0x20, 0x53,
//     0x6F, 0x20, 0x74, 0x68, 0x65, 0x20, 0x46, 0x6F, 0x78, 0x2C, 0x20,
//     0x77, 0x68, 0x6F, 0x20, 0x68, 0x61, 0x64, 0x20, 0x63, 0x6F, 0x6E,
//     0x73, 0x65, 0x6E, 0x74, 0x65, 0x64, 0x20, 0x74, 0x6F, 0x20, 0x61,
//     0x63, 0x74, 0x20, 0x61, 0x73, 0x20, 0x6A, 0x75, 0x64, 0x67, 0x65,
//     0x2C, 0x20, 0x6D, 0x61, 0x72, 0x6B, 0x65, 0x64, 0x20, 0x74, 0x68,
//     0x65, 0x20, 0x64, 0x69, 0x73, 0x74, 0x61, 0x6E, 0x63, 0x65, 0x20,
//     0x61, 0x6E, 0x64, 0x20, 0x73, 0x74, 0x61, 0x72, 0x74, 0x65, 0x64,
//     0x20, 0x74, 0x68, 0x65, 0x20, 0x72, 0x75, 0x6E, 0x6E, 0x65, 0x72,
//     0x73, 0x20, 0x6F, 0x66, 0x66, 0x2E, 0x0D, 0x0A, 0x54, 0x68, 0x65,
//     0x20, 0x48, 0x61, 0x72, 0x65, 0x20, 0x77, 0x61, 0x73, 0x20, 0x73,
//     0x6F, 0x6F, 0x6E, 0x20, 0x66, 0x61, 0x72, 0x20, 0x6F, 0x75, 0x74,
//     0x20, 0x6F, 0x66, 0x20, 0x73, 0x69, 0x67, 0x68, 0x74, 0x2C, 0x20,
//     0x61, 0x6E, 0x64, 0x20, 0x74, 0x6F, 0x20, 0x6D, 0x61, 0x6B, 0x65,
//     0x20, 0x74, 0x68, 0x65, 0x20, 0x54, 0x6F, 0x72, 0x74, 0x6F, 0x69,
//     0x73, 0x65, 0x20, 0x66, 0x65, 0x65, 0x6C, 0x20, 0x76, 0x65, 0x72,
//     0x79, 0x20, 0x64, 0x65, 0x65, 0x70, 0x6C, 0x79, 0x20, 0x68, 0x6F,
//     0x77, 0x20, 0x72, 0x69, 0x64, 0x69, 0x63, 0x75, 0x6C, 0x6F, 0x75,
//     0x73, 0x20, 0x69, 0x74, 0x20, 0x77, 0x61, 0x73, 0x20, 0x66, 0x6F,
//     0x72, 0x20, 0x68, 0x69, 0x6D, 0x20, 0x74, 0x6F, 0x20, 0x74, 0x72,
//     0x79, 0x20, 0x61, 0x20, 0x72, 0x61, 0x63, 0x65, 0x20, 0x77, 0x69,
//     0x74, 0x68, 0x20, 0x61, 0x20, 0x48, 0x61, 0x72, 0x65, 0x2C, 0x20,
//     0x68, 0x65, 0x20, 0x6C, 0x61, 0x79, 0x20, 0x64, 0x6F, 0x77, 0x6E,
//     0x20, 0x62, 0x65, 0x73, 0x69, 0x64, 0x65, 0x20, 0x74, 0x68, 0x65,
//     0x20, 0x63, 0x6F, 0x75, 0x72, 0x73, 0x65, 0x20, 0x74, 0x6F, 0x20,
//     0x74, 0x61, 0x6B, 0x65, 0x20, 0x61, 0x20, 0x6E, 0x61, 0x70, 0x20,
//     0x75, 0x6E, 0x74, 0x69, 0x6C, 0x20, 0x74, 0x68, 0x65, 0x20, 0x54,
//     0x6F, 0x72, 0x74, 0x6F, 0x69, 0x73, 0x65, 0x20, 0x73, 0x68, 0x6F,
//     0x75, 0x6C, 0x64, 0x20, 0x63, 0x61, 0x74, 0x63, 0x68, 0x20, 0x75,
//     0x70, 0x2E, 0x20, 0x0D, 0x0A};

void filesDisplay(void) {
  epaperDisplayFiles(
      &filesStates,
      &(filesHomescreenDisplayLocations[filesStates.filesHomescreenState]));
}

// void filesDisplayLength(uint32_t textLength) {
//   epaperDisplayTextFile(someBuf, textLength);
// }

// void filesEmptyTheBuffer(void) {
//   sendreceivePushToSpi();
// }

// char emptyBuffer[100] = {0};
// void filesTest(void) {
//   W25qxx_EraseBlock(0);
//   // W25qxx_EraseBlock(1);
//   // uint8_t byte = 0x65;
//   uint8_t pageOffset = 0;
//   uint8_t pageNum = 0;
//   // W25qxx_WritePage(&byte, pageNum, pageOffset, 1);

//   char testString[] = {"Stringtotest"};

//   W25qxx_WriteBlock((uint8_t*)testString, pageNum, pageOffset,
//                     sizeof(testString));
//   W25qxx_ReadBlock((uint8_t*)emptyBuffer, pageNum, pageOffset,
//                    sizeof(testString));

//   for (uint32_t i = 0; i < sizeof(testString); i++) {
//     if (testString[i] != emptyBuffer[i]) {
//       epaperDisplayError(201);
//       // epaperDisplayError(emptyBuffer[i]);

//       SWERROR_HANDLER();
//     }
//   }

//   epaperDisplayError(3006);
// }

// Global Press
void filesHandleTopLeftPress(void) {
  (*(filesButtonHandlersList[filesStates.filesGlobalState]
         ->topLeftButtonPress))();
}

void filesHandleTopRightPress(void) {
  (*(filesButtonHandlersList[filesStates.filesGlobalState]
         ->topRightButtonPress))();
}

void filesHandleBottomRightPress(void) {
  (*(filesButtonHandlersList[filesStates.filesGlobalState]
         ->bottomRightButtonPress))();
}

void filesHandleBottomLeftPress(void) {
  (*(filesButtonHandlersList[filesStates.filesGlobalState]
         ->bottomLeftButtonPress))();
}

// Homescreen Presses
// Move highlight down
static void filesHomescreenHandleTopRightPress(void) {
  if ((filesStates.filesHomescreenState + 1) == filesHomescreenStateSize)
    filesStates.filesHomescreenState = filesHomescreenDirectory;
  else
    filesStates.filesHomescreenState = filesHomescreenUpload;

  epaperDisplayFiles(
      &filesStates,
      &(filesHomescreenDisplayLocations[filesStates.filesHomescreenState]));
}

// select
static void filesHomescreenHandleBottomRightPress(void) {
  filesStates.filesGlobalState = filesResolveHomescreenSelect();
  filesStates.filesHomescreenState = filesDefaultStates.filesHomescreenState;
  // epaperDisplayFiles(&filesStates, NULL);  // maybe just do this in
  // sendreceive
  if (filesStates.filesGlobalState == filesGlobalUpload)
    sendReceiveStartUart();
  else if (filesStates.filesGlobalState == filesGlobalDirectory)
    filesDisplayDirectoryFilenames(filesDirectoryHighlightStay);
  else
    SWERROR_HANDLER();
}

static void filesHomescreenHandleBottomLeftPress(void) {
  filesStates = filesDefaultStates;
  epaperExitToHome();
}

// Upload Presses
// exit upload
static void filesUploadHandleBottomLeftPress(void) {
  sendReceiveStopUart();
  filesStates.filesGlobalState = filesGlobalHomescreen;
  filesStates.filesUploadingState = filesDefaultStates.filesUploadingState;
  epaperDisplayFiles(
      &filesStates,
      &(filesHomescreenDisplayLocations[filesStates.filesHomescreenState]));
}

void filesDisplayUartProcessing(void) {
  filesStates.filesUploadingState = filesUploading;
  epaperDisplayFiles(&filesStates, &filesUartPrintPosition);
}

void filesDisplayUartNothingSent(void) {
  filesStates.filesUploadingState = filesUploadingNothingSent;
  epaperDisplayFiles(&filesStates, &filesUartPrintPosition);
}

void filesDisplayUartDone(void) {
  filesStates.filesUploadingState = filesUploadingDone;
  epaperDisplayFiles(&filesStates, &filesUartPrintPosition);
}

static const ButtonHandlerSetup filesDirectoryTitleButtonHandlers = {
    .topLeftButtonPress = filesDirectoryTitleTopLeftPress,
    .topRightButtonPress = filesDirectoryTitleTopRightPress,
    .bottomLeftButtonPress = filesDirectoryTitleBottomLeftPress,
    .bottomRightButtonPress = filesDirectoryTitleBottomRightPress};

static const ButtonHandlerSetup filesDirectoryTextButtonHandlers = {
    .topLeftButtonPress = emptyFunction,
    .topRightButtonPress = filesDirectoryTextTopRightPress,
    .bottomLeftButtonPress = filesDirectoryTextBottomLeftPress,
    .bottomRightButtonPress = emptyFunction};

static ButtonHandlerSetup const* const
    filesDirectoryButtonHandlersList[filesDirectoryStatesSize] = {
        &filesDirectoryTitleButtonHandlers, &filesDirectoryTextButtonHandlers};

// Directory Presses
static void filesDirectoryHandleTopLeftPress(void) {
  (*(filesDirectoryButtonHandlersList[filesStates.filesDirectoryState]
         ->topLeftButtonPress))();
}

static void filesDirectoryHandleTopRightPress(void) {
  (*(filesDirectoryButtonHandlersList[filesStates.filesDirectoryState]
         ->topRightButtonPress))();
}

static void filesDirectoryHandleBottomRightPress(void) {
  (*(filesDirectoryButtonHandlersList[filesStates.filesDirectoryState]
         ->bottomRightButtonPress))();
}

static void filesDirectoryHandleBottomLeftPress(void) {
  (*(filesDirectoryButtonHandlersList[filesStates.filesDirectoryState]
         ->bottomLeftButtonPress))();
}

// Directory Title State
// move up
static FilesDirectoryHighlight_e filesDirectoryHighlight = filesTitleHighlight1;
static void filesDirectoryTitleTopLeftPress(void) {
  if (filesDirectoryHighlight == filesTitleHighlight1) {
    // filesDirectoryHighlight = filesTitleHighlight3;
    filesDisplayDirectoryFilenamesPageUp();
  } else {
    filesDisplayDirectoryFilenames(filesDirectoryHighlightUp);
  }
}
// movedown
static void filesDirectoryTitleTopRightPress(void) {
  if (filesDirectoryHighlight == filesTitleHighlight3) {
    // filesDirectoryHighlight = filesTitleHighlight1;
    filesDisplayDirectoryFilenamesPageDown();
  } else {
    filesDisplayDirectoryFilenames(filesDirectoryHighlightDown);
  }
}
// select
static void filesDirectoryTitleBottomRightPress(void) {
  filesStates.filesDirectoryState = filesDirectoryText;
  filesDisplayDirectoryText();
}
// exit
static int32_t filesDirectoryOffset = 2;
static void filesDirectoryTitleBottomLeftPress(void) {
  filesStates.filesGlobalState = filesGlobalHomescreen;
  filesStates.filesDirectoryState = filesDefaultStates.filesDirectoryState;
  filesDirectoryOffset = 2;
  filesDirectoryHighlight = filesTitleHighlight1;
  epaperDisplayFiles(
      &filesStates,
      &(filesHomescreenDisplayLocations[filesStates.filesHomescreenState]));
}

static FilesGlobalStates_e filesResolveHomescreenSelect(void) {
  FilesGlobalStates_e state;
  switch (filesStates.filesHomescreenState) {
    case (filesHomescreenDirectory): {
      state = filesGlobalDirectory;
      break;
    }
    case (filesHomescreenUpload): {
      state = filesGlobalUpload;
      break;
    }
    default:
      SWERROR_HANDLER();
  }
  return state;
}

// char textFile[512];
// char fileNmthing[128];
// static void filesDisplayDirectoryFilenames(void) {
//   int err = lfs_dir_open(&lfs, &dir, mainFolderName);
//   if (err)
//     filesDisplayErrorTesting("opendir", err);

//   for (int i = 0; i < 3; i++) {
//     int err2 = lfs_dir_read(&lfs, &dir, &info);
//     if (err2 < 0)
//       filesDisplayErrorTesting("readdir", err2);
//   }
//   snprintf(fileNmthing, sizeof(fileNmthing), "/MainFolder/%s", info.name);
//   err = lfs_file_open(&lfs, &file, fileNmthing, LFS_O_RDONLY);
//   if (err)
//     filesDisplayErrorTesting("fopen", err);
//   int err3 = lfs_file_read(&lfs, &file, (void*)textFile, sizeof(textFile));
//   if (err3 < 0)
//     filesDisplayErrorTesting("Textfile", err3);
//   // epaperDisplayTestingString(info.name);
//   epaperDisplayTextFile((uint8_t const* const)textFile, sizeof(textFile));
//   err = lfs_file_close(&lfs, &file);
//   if (err)
//     filesDisplayErrorTesting("fclose", err);
//   // }
//   int err1 = lfs_dir_close(&lfs, &dir);
//   if (err1)
//     filesDisplayErrorTesting("closedir", err1);
// }

lfs_dir_t dir;
struct lfs_info info;
char* filesFileNamesPtr[DIRNUMTITLES];
char filesFileNames[DIRNUMTITLES][FILESMAXTITLESIZE];
static void filesDisplayDirectoryFilenames(
    FilesDirectoryHighlightDirection_e highlightDirection) {
  memset(filesFileNames, 0, sizeof filesFileNames);
  FilesDirectoryHighlight_e tempHighlight =
      filesHandleHighlightChange(highlightDirection);

  lfs_dir_open(&lfs, &dir, mainFolderName);
  // dir.pos = filesDirectoryOffset;
  lfs_dir_seek(&lfs, &dir, filesDirectoryOffset);
  // for (uint32_t i = 0; i < 6; i++) {
  //   lfs_dir_read(&lfs, &dir, &info);
  //   filesDisplayErrorTesting(info.name, i);
  //   nrf_delay_ms(500);
  // }
  for (uint32_t i = 0; i < DIRNUMTITLES; i++) {
    int readRet = lfs_dir_read(&lfs, &dir, &info);
    if (readRet == 0) {
      filesFileNamesPtr[i] = NULL;
    } else {
      if (i == tempHighlight)
        filesDirectoryHighlight = tempHighlight;
      snprintf(&(filesFileNames[i][0]), FILESMAXTITLESIZE, "%s", info.name);
      filesFileNamesPtr[i] = &(filesFileNames[i][0]);
    }
  }
  lfs_dir_close(&lfs, &dir);
  epaperDisplayFiles(&filesStates, NULL);
}

static void filesDisplayDirectoryFilenamesPageUp(void) {
  if ((filesDirectoryOffset - DIRNUMTITLES) < 2) {
    filesDirectoryOffset = 2;
    return;
  } else {
    filesDirectoryOffset -= DIRNUMTITLES;
  }
  filesDisplayDirectoryFilenames(filesDirectoryHighlightUp);
}

static void filesDisplayDirectoryFilenamesPageDown(void) {
  uint32_t tempOffset = filesDirectoryOffset;

  lfs_dir_open(&lfs, &dir, mainFolderName);
  tempOffset += DIRNUMTITLES;
  // dir.pos = tempOffset;

  int err = lfs_dir_seek(&lfs, &dir, tempOffset);
  // lfs_dir_read(&lfs, &dir, &info);
  // filesDisplayErrorTesting(info.name, dir.pos);
  // while (1)
  //   ;
  if (err < 0) {
    lfs_dir_close(&lfs, &dir);
    return;
  }
  int readRet = lfs_dir_read(&lfs, &dir, &info);
  if (readRet <= 0) {
    lfs_dir_close(&lfs, &dir);
    return;
  } else {
    filesDirectoryOffset = tempOffset;
  }
  lfs_dir_close(&lfs, &dir);
  filesDisplayDirectoryFilenames(filesDirectoryHighlightDown);
}

const FilesDirectoryHighlight_e filesGetDirectoryFilenamesHighlight(void) {
  return filesDirectoryHighlight;
}

char const* const filesGetDirectoryFilenames(uint32_t index) {
  if (filesFileNamesPtr[index] == NULL) {
    return NULL;
  } else {
    return (filesFileNamesPtr[index]);
  }
}

char textFile[FILESTEXTFILEMAXSIZE];
static char filePathBuffer[FILESMAXTITLESIZE * 3];
static uint32_t nextOffset = 0;
static uint32_t fileSize = 0;
static void filesDisplayDirectoryText(void) {
  // get file from offset + highlight
  lfs_dir_open(&lfs, &dir, mainFolderName);
  lfs_dir_seek(&lfs, &dir, filesDirectoryOffset + filesDirectoryHighlight);
  lfs_dir_read(&lfs, &dir, &info);
  snprintf(filePathBuffer, sizeof(filePathBuffer), "/MainFolder/%s", info.name);
  fileSize = info.size;
  lfs_dir_close(&lfs, &dir);
  filesTextFileDisplay();
}

// Directory text state
// pageup
// static void filesDirectoryTextTopLeftPress(void) {}

// page down
static void filesDirectoryTextTopRightPress(void) {
  filesTextFileDisplay();
}

// static void filesDirectoryTextBottomRightPress(void) {}

// exit
static int32_t bytesRead = 0;
static void filesDirectoryTextBottomLeftPress(void) {
  filesStates.filesDirectoryState = filesDirectoryFileNames;
  nextOffset = 0;
  bytesRead = 0;
  fileSize = 0;
  filesDisplay();
}

static void filesTextFileDisplay(void) {
  if (nextOffset >= fileSize) {
    return;
  }
  memset(textFile, 0, sizeof textFile);
  lfs_file_open(&lfs, &file, filePathBuffer, LFS_O_RDONLY);
  int32_t err = lfs_file_seek(&lfs, &file, nextOffset, 0);
  if (err < 0) {
    lfs_file_close(&lfs, &file);
    return;
  }
  bytesRead = lfs_file_read(&lfs, &file, (void*)textFile, sizeof(textFile));
  if (bytesRead < 0) {
    lfs_file_close(&lfs, &file);
    return;
  }

  lfs_file_close(&lfs, &file);
  epaperDisplayFiles(&filesStates, NULL);
  nextOffset += (uint32_t)(epaperGetNewTextFileLocation() - textFile);
}

const FilesTextFilePackage filesGetTextFile(void) {
  FilesTextFilePackage textFilePackage = {textFile, bytesRead};
  return textFilePackage;
}

static FilesDirectoryHighlight_e filesHandleHighlightChange(
    FilesDirectoryHighlightDirection_e highlightDirection) {
  FilesDirectoryHighlight_e tempHighlight = filesDirectoryHighlight;

  switch (highlightDirection) {
    case (filesDirectoryHighlightUp): {
      tempHighlight = (tempHighlight == filesTitleHighlight1)
                          ? filesTitleHighlight3
                          : tempHighlight - 1;
      break;
    }
    case (filesDirectoryHighlightDown): {
      tempHighlight = (tempHighlight == filesTitleHighlight3)
                          ? filesTitleHighlight1
                          : tempHighlight + 1;
      break;
    }
    case (filesDirectoryHighlightStay):
      break;
  }
  return tempHighlight;
}

static void filesDisplayErrorTesting(char const* text, int errorNum) {
  char str[20];
  sprintf(str, "%s %d", text, errorNum);
  epaperDisplayTestingString(str);
  // SWERROR_HANDLER();
}

// Takes a buffer and size indicating where to store the read data.
// Returns the number of bytes read, or a negative error code on failure.
lfs_ssize_t lfs_file_read(lfs_t* lfs,
                          lfs_file_t* file,
                          void* buffer,
                          lfs_size_t size);