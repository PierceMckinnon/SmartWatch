#ifndef _FILES_H_
#define _FILES_H_

#include "main.h"

#include "lfs.h"

#define UPLOADX 45
#define UPLOADY 39
#define DIRECTORYX 45
#define DIRECTORYY 117
#define FILESRECWIDTH 110
#define FILESRECHEIGHT 45

#define UARTX 10
#define UARTY 20

#define DIRNUMTITLES 3
#define DIRTITLEX 25
#define DIRTITLEY 12
#define DIRYOFFSET 60
#define DIRSELECTRECX 9
#define DIRSELECRECHEIGHT 16
#define DIRSELECRECWIDTH 11

#define FILESMAXTITLESIZE 36
#define FILESTEXTFILEMAXSIZE 512

typedef enum FilesGlobalStates_e {
  filesGlobalHomescreen,
  filesGlobalDirectory,
  filesGlobalUpload,
  filesGlobalStatesSize
} FilesGlobalStates_e;

typedef enum FilesHomescreenStates_e {
  filesHomescreenDirectory,
  filesHomescreenUpload,
  filesHomescreenStateSize
} FilesHomescreenStates_e;

typedef enum FilesDirectoryStates_e {
  filesDirectoryFileNames,
  filesDirectoryText,
  filesDirectoryStatesSize
} FilesDirectoryStates_e;

typedef enum FilesUploadingStates_e {
  filesUploading,
  filesUploadingDone,
  filesUploadingNothingSent,
  filesUploadingStatesSize
} FilesUploadingStates_e;

typedef enum FilesDirectoryHighlight_e {
  filesTitleHighlight1,
  filesTitleHighlight2,
  filesTitleHighlight3
} FilesDirectoryHighlight_e;

typedef enum FilesDirectoryHighlightDirection_e {
  filesDirectoryHighlightStay,
  filesDirectoryHighlightUp,
  filesDirectoryHighlightDown
} FilesDirectoryHighlightDirection_e;

typedef struct FilesStatesTracker {
  FilesGlobalStates_e filesGlobalState;
  FilesHomescreenStates_e filesHomescreenState;
  FilesDirectoryStates_e filesDirectoryState;
  FilesUploadingStates_e filesUploadingState;
} FilesStatesTracker;

extern const ButtonHandlerSetup filesButtonHandlers;
void filesHandleTopLeftPress(void);
void filesHandleTopRightPress(void);
void filesHandleBottomLeftPress(void);
void filesHandleBottomRightPress(void);

void filesDisplay(void);
void filesDisplayLength(uint32_t textLength);
// void filesEmptyTheBuffer(void);
// void filesTest(void);
void filesFSInit(void);

int filesReadWrapper(const struct lfs_config* c,
                     lfs_block_t block,
                     lfs_off_t off,
                     void* buffer,
                     lfs_size_t size);
int filesWriteWrapper(const struct lfs_config* c,
                      lfs_block_t block,
                      lfs_off_t off,
                      const void* buffer,
                      lfs_size_t size);
int filesEraseWrapper(const struct lfs_config* c, lfs_block_t block);

int filesSyncWrapper(const struct lfs_config* c);

extern const ButtonHandlerSetup filesButtonHandlers;

void filesHandleTopLeftPress(void);
void filesHandleTopRightPress(void);
void filesHandleBottomRightPress(void);
void filesHandleBottomLeftPress(void);

void filesDisplayUartProcessing(void);
void filesDisplayUartNothingSent(void);
void filesDisplayUartDone(void);

void filesFileOpenForWrite(char const* title);
void filesFileWrite(uint8_t* textFile, uint32_t sizeToWrite);
void filesFileClose(void);

typedef struct FilesTextFilePackage {
  char const* const textFile;
  const uint32_t size;
} FilesTextFilePackage;
const FilesTextFilePackage filesGetTextFile(void);

char const* const filesGetDirectoryFilenames(uint32_t index);
const FilesDirectoryHighlight_e filesGetDirectoryFilenamesHighlight(void);
#endif  //_FILES_H_