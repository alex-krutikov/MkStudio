Name       "MKStudio"
OutFile    "..\..\set\mkstudio-bin-update.exe"
InstallDir $PROGRAMFILES\miksys\umikon\mkstudio

XpStyle on
SetCompressor lzma

LoadLanguageFile "${NSISDIR}\Contrib\Language files\Russian.nlf"

InstallDirRegKey HKLM "Software\MKStudio" "Install_Dir"

;--------------------------------

; Pages

Page components
Page directory
Page instfiles

UninstPage instfiles

;--------------------------------

; The stuff to install
Section "Обновление MKStudio и MKView"
  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put file there
  File ..\..\bin\MKView.exe
  File ..\..\bin\MKStudio.exe
  File ..\..\bin\MKServer.exe
  File ..\..\bin\mkserverd.exe
  File ..\..\bin\mkquery.exe
  File ..\..\bin\mksync.exe
  
SectionEnd
