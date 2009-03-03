Name       "MKView"
OutFile    "../../set/mkview-setup.exe"
InstallDir $PROGRAMFILES\miksys\umikon\mkview

XpStyle on
;SetCompressor lzma

LoadLanguageFile "${NSISDIR}\Contrib\Language files\Russian.nlf"

InstallDirRegKey HKLM "Software\MKView" "Install_Dir"

;--------------------------------

; Pages

Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

;--------------------------------

; The stuff to install
Section "MKView"
  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put file there
  File ..\..\bin\mkview.exe
  File ..\..\bin\mklib.dll
  File ..\..\bin\*.xml

  File ..\..\..\qt\bin\qtcore4.dll
  File ..\..\..\qt\bin\qtgui4.dll
  File ..\..\..\qt\bin\qtxml4.dll
  File ..\..\..\qt\bin\mingwm10.dll
  File ..\..\..\qt\bin\qwt5.dll
  
  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\MKView "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\MKView" "DisplayName" "MKView"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\MKView" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\MKView" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\MKView" "NoRepair" 1
  WriteUninstaller "uninstall.exe"

  CreateDirectory "$SMPROGRAMS\MikSYS\MKView"
  CreateShortCut "$SMPROGRAMS\MikSYS\MKView\”даление.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortCut "$SMPROGRAMS\MikSYS\MKView\MKView.lnk" "$INSTDIR\MKView.exe" "" "$INSTDIR\MKView.exe" 0
SectionEnd

; Optional section (can be disabled by the user)
Section "»конка на рабочий стол"
  CreateShortCut "$DESKTOP\MKView.lnk" "$INSTDIR\MKView.exe" "" "$INSTDIR\MKView.exe" 0
SectionEnd

;----------------------------------------------------------------------------------------------------------------

; Uninstaller

Section "Uninstall"
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\MKView"
  DeleteRegKey HKLM SOFTWARE\MKView

  ; Remove files and uninstaller
  Delete $INSTDIR\*.*

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\MikSYS\MKView\*.*"
  Delete "$DESKTOP\MKView.lnk"

  ; Remove directories used
  RMDir "$SMPROGRAMS\MikSYS\MKView"
  RMDir "$INSTDIR"

SectionEnd
