Unicode true

Name       "MKStudio"
OutFile    "mkstudio-setup.exe"
InstallDir $PROGRAMFILES\mkstudio

XpStyle on
SetCompressor lzma

LoadLanguageFile "${NSISDIR}\Contrib\Language files\Russian.nlf"

InstallDirRegKey HKLM "Software\MKStudio" "Install_Dir"

LicenseData gpl_license.txt

;--------------------------------

; Pages

Page license
Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

;--------------------------------

; The stuff to install
Section "MKStudio и MKView"
  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put file there
  File ${INSTALL_ROOT}\baseedit.exe
  File ${INSTALL_ROOT}\mkview.exe
  File ${INSTALL_ROOT}\mkstudio.exe
  File ${INSTALL_ROOT}\mkstudio-arduino.exe
  File ${INSTALL_ROOT}\mkserver.exe
  File ${INSTALL_ROOT}\mkserverd.exe
  File ${INSTALL_ROOT}\mkquery.exe
  File ${INSTALL_ROOT}\mksync.exe
  File ${INSTALL_ROOT}\mmpm.exe

  File ${QT_DLL_DIR}\libgcc_s_dw2-1.dll
  File ${QT_DLL_DIR}\libwinpthread-1.dll
  File ${QT_DLL_DIR}\libstdc++-6.dll
  File ${QT_DLL_DIR}\Qt5Core.dll
  File ${QT_DLL_DIR}\Qt5Gui.dll
  File ${QT_DLL_DIR}\Qt5Widgets.dll
  File ${QT_DLL_DIR}\Qt5Network.dll
  File ${QT_DLL_DIR}\Qt5Xml.dll

  SetOutPath $INSTDIR\platforms
  File ${QT_DLL_DIR}\..\plugins\platforms\qwindows.dll


  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\MKStudio "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\MKStudio" "DisplayName" "MKStudio"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\MKStudio" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\MKStudio" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\MKStudio" "NoRepair" 1
  WriteUninstaller "uninstall.exe"

  CreateDirectory "$SMPROGRAMS\MKStudio"
  CreateShortCut "$SMPROGRAMS\MKStudio\Удаление.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortCut "$SMPROGRAMS\MKStudio\BaseEdit.lnk" "$INSTDIR\baseedit.exe"  "" "$INSTDIR\baseedit.exe" 0
  CreateShortCut "$SMPROGRAMS\MKStudio\MKView.lnk"   "$INSTDIR\MKView.exe"    "" "$INSTDIR\MKView.exe" 0
  CreateShortCut "$SMPROGRAMS\MKStudio\MKStudio.lnk" "$INSTDIR\MKStudio.exe"  "" "$INSTDIR\MKStudio.exe" 0
  CreateShortCut "$SMPROGRAMS\MKStudio\MKServer.lnk" "$INSTDIR\MKServer.exe"  "" "$INSTDIR\MKServer.exe" 0
  CreateShortCut "$SMPROGRAMS\MKStudio\mmpm.lnk"     "$INSTDIR\mmpm.exe"      "" "$INSTDIR\mmpm.exe" 0
SectionEnd

; Optional section (can be disabled by the user)
Section "Файлы проектов для МК Mikkon"
  SetOutPath $INSTDIR\mikkon
  File ..\..\bin\mikkon\*.xml
SectionEnd

SectionGroup /e "Иконки на рабочий стол"
Section "MKView"
  CreateShortCut "$DESKTOP\MKView.lnk"   "$INSTDIR\MKView.exe"   "" "$INSTDIR\MKView.exe"   0
SectionEnd
Section "MKStudio"
  CreateShortCut "$DESKTOP\MKStudio.lnk" "$INSTDIR\MKStudio.exe" "" "$INSTDIR\MKStudio.exe" 0
SectionEnd
Section "MMPM"
  CreateShortCut "$DESKTOP\mmpm.lnk"     "$INSTDIR\mmpm.exe"     "" "$INSTDIR\mmpm.exe"     0
SectionEnd
SectionGroupEnd

;Section "Иконки на рабочий стол"
;  CreateShortCut "$DESKTOP\BaseEdit.lnk" "$INSTDIR\baseedit.exe" "" "$INSTDIR\baseedit.exe" 0
;  CreateShortCut "$DESKTOP\MKStudio.lnk" "$INSTDIR\MKStudio.exe" "" "$INSTDIR\MKStudio.exe" 0
;  CreateShortCut "$DESKTOP\MKView.lnk"   "$INSTDIR\MKView.exe"   "" "$INSTDIR\MKView.exe"   0
;  CreateShortCut "$DESKTOP\MKServer.lnk" "$INSTDIR\MKServer.exe" "" "$INSTDIR\MKServer.exe" 0
;  CreateShortCut "$DESKTOP\mmpm.lnk"     "$INSTDIR\mmpm.exe"     "" "$INSTDIR\mmpm.exe"     0
;SectionEnd

;----------------------------------------------------------------------------------------------------------------

; Uninstaller

Section "Uninstall"
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\MKStudio"
  DeleteRegKey HKLM SOFTWARE\MKStudio

  ; Remove files and uninstaller
  Delete $INSTDIR\mikkon\*.xml

  Delete $INSTDIR\baseedit.exe
  Delete $INSTDIR\mkview.exe
  Delete $INSTDIR\mkstudio.exe
  Delete $INSTDIR\mkserver.exe
  Delete $INSTDIR\mkserverd.exe
  Delete $INSTDIR\mkquery.exe
  Delete $INSTDIR\mksync.exe
  Delete $INSTDIR\mmpm.exe

  Delete $INSTDIR\libgcc_s_dw2-1.dll
  Delete $INSTDIR\libwinpthread-1.dll
  Delete $INSTDIR\libstdc++-6.dll
  Delete $INSTDIR\Qt5Core.dll
  Delete $INSTDIR\Qt5Gui.dll
  Delete $INSTDIR\Qt5Widgets.dll
  Delete $INSTDIR\Qt5Network.dll
  Delete $INSTDIR\Qt5Xml.dll

  Delete $INSTDIR\platforms\qwindows.dll

  Delete $INSTDIR\uninstall.exe

  Delete $PROFILE\.baseedit_config
  Delete $PROFILE\.mkstudio_config
  Delete $PROFILE\.mkview_config
  Delete $PROFILE\.mkserver_config
  Delete $PROFILE\.mmpm_config

  Delete $INSTDIR\classic_registers.xml

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\MKStudio\*.*"
  Delete "$DESKTOP\BaseEdit.lnk"
  Delete "$DESKTOP\MKStudio.lnk"
  Delete "$DESKTOP\MKView.lnk"
  Delete "$DESKTOP\MKServer.lnk"
  Delete "$DESKTOP\mmpm.lnk"

  ; Remove directories used
  RMDir "$SMPROGRAMS\MKStudio"
  RMDir "$INSTDIR\platforms"
  RMDir "$INSTDIR\mikkon"
  RMDir "$INSTDIR"

SectionEnd
