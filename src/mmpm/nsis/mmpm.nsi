Name       "MMpM"
OutFile    "mmpm-setup.exe"
InstallDir $PROGRAMFILES\MikSYS\UMIKON\MMpM

XpStyle on
SetCompressor lzma

LoadLanguageFile "${NSISDIR}\Contrib\Language files\Russian.nlf"

InstallDirRegKey HKLM "Software\MMpM" "Install_Dir"

Caption           "Установка MMpM"
DirText           "Установка менеджера микропрограмм модулей ПТК УМИКОН$\nСборка инсталляции от ${__DATE__}" "Каталог установки" "Выбрать..."
ComponentText     "Менеджер микропрограмм модулей ПТК УМИКОН$\nСборка инсталляции от ${__DATE__}"
BrandingText      /TRIMCENTER "1994-2021 (С) УМИКОН"
Icon              "miksys.ico"
; CheckBitmap       "..\src\grey-cross.bmp"


;--------------------------------

; Pages

Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

;--------------------------------

; The stuff to install
Section "MMpM"
  SectionIn RO

  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put file there
  File mmpm.exe

  File libgcc_s_seh-1.dll
  File libstdc++-6.dll
  File libwinpthread-1.dll

  File Qt5Core.dll
  File Qt5Gui.dll
  File Qt5Network.dll
  File Qt5Widgets.dll
  File Qt5Xml.dll

  SetOutPath $INSTDIR\platforms
  File platforms\qwindows.dll

  SetOutPath $INSTDIR\styles
  File styles\qwindowsvistastyle.dll 

  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\MMpM "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\MMpM" "DisplayName" "MMpM"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\MMpM" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\MMpM" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\MMpM" "NoRepair" 1
  WriteUninstaller "uninstall.exe"

  CreateDirectory "$SMPROGRAMS\MMpM"
  CreateShortCut  "$SMPROGRAMS\MMpM\Удаление.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortCut  "$SMPROGRAMS\MMpM\MMpM.lnk" "$INSTDIR\MMpM.exe" "" "$INSTDIR\MMpM.exe" 0
SectionEnd

; Optional section (can be disabled by the user)
Section "Иконка на рабочий стол"
  CreateShortCut "$DESKTOP\MMpM.lnk" "$INSTDIR\MMpM.exe" "" "$INSTDIR\MMpM.exe" 0
SectionEnd

;----------------------------------------------------------------------------------------------------------------

; Uninstaller

Section "Uninstall"
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\MMpM"
  DeleteRegKey HKLM SOFTWARE\MMpM

  RMDir /r "$INSTDIR"

  ; Remove shortcuts, if any
  Delete "$DESKTOP\MMpM.lnk"

  ; Remove directories used
  RMDir /r "$SMPROGRAMS\MMpM"

SectionEnd
