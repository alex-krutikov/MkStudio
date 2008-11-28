Name       "MKStudio"
OutFile    "..\..\set\mkstudio-setup.exe"
InstallDir $PROGRAMFILES\miksys\umikon\mkstudio

XpStyle on
;SetCompressor lzma

LoadLanguageFile "${NSISDIR}\Contrib\Language files\Russian.nlf"

InstallDirRegKey HKLM "Software\MKStudio" "Install_Dir"

;--------------------------------

; Pages

Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

;--------------------------------

; The stuff to install
Section "MKStudio � MKView"
  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put file there
  File ..\..\bin\MKView.exe
  File ..\..\bin\MKStudio.exe
  File ..\..\bin\mklib.dll
  File ..\..\bin\mkview_*.dll


  File ..\..\..\qt\bin\qtcore4.dll
  File ..\..\..\qt\bin\qtgui4.dll
  File ..\..\..\qt\bin\qtxml4.dll
  File ..\..\..\qt\bin\mingwm10.dll
  File ..\..\..\qt\bin\qwt5.dll
  
  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\MKStudio "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\MKStudio" "DisplayName" "MKStudio"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\MKStudio" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\MKStudio" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\MKStudio" "NoRepair" 1
  WriteUninstaller "uninstall.exe"

  CreateDirectory "$SMPROGRAMS\MikSYS\MKStudio"
  CreateShortCut "$SMPROGRAMS\MikSYS\MKStudio\��������.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortCut "$SMPROGRAMS\MikSYS\MKStudio\MKView.lnk"   "$INSTDIR\MKView.exe"    "" "$INSTDIR\MKView.exe" 0
  CreateShortCut "$SMPROGRAMS\MikSYS\MKStudio\MKStudio.lnk" "$INSTDIR\MKStudio.exe" "" "$INSTDIR\MKStudio.exe" 0
SectionEnd

; Optional section (can be disabled by the user)
Section "����� ��������"
  File ..\..\bin\*.xml
SectionEnd

Section "������ �� ������� ����"
  CreateShortCut "$DESKTOP\MKStudio.lnk" "$INSTDIR\MKStudio.exe" "" "$INSTDIR\MKStudio.exe" 0
  CreateShortCut "$DESKTOP\MKView.lnk"   "$INSTDIR\MKView.exe"   "" "$INSTDIR\MKView.exe"   0
SectionEnd

;----------------------------------------------------------------------------------------------------------------

; Uninstaller

Section "Uninstall"
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\MKStudio"
  DeleteRegKey HKLM SOFTWARE\MKStudio

  ; Remove files and uninstaller
  Delete $INSTDIR\*.*

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\MikSYS\MKStudio\*.*"
  Delete "$DESKTOP\MKStudio.lnk"
  Delete "$DESKTOP\MKView.lnk"


  ; Remove directories used
  RMDir "$SMPROGRAMS\MikSYS\MKStudio"
  RMDir "$INSTDIR"

SectionEnd
