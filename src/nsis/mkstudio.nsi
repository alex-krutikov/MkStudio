Name       "MKStudio"
OutFile    "${MKROOT}\set\mkstudio-setup.exe"
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

UninstPage uninstConfirm
UninstPage instfiles

;--------------------------------

; The stuff to install
Section "MKStudio � MKView"
  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put file there
  File ${MKROOT}\bin\MKView.exe
  File ${MKROOT}\bin\MKStudio.exe
  File ${MKROOT}\bin\MKServer.exe
  File ${MKROOT}\bin\mkserverd.exe
  File ${MKROOT}\bin\mkquery.exe
  File ${MKROOT}\bin\mksync.exe
  File ${MKROOT}\bin\classic_registers.xml

!ifdef QT_DLL_DIR
  File ${QT_DLL_DIR}\qtcore4.dll
  File ${QT_DLL_DIR}\qtgui4.dll
  File ${QT_DLL_DIR}\qtxml4.dll
  File ${QT_DLL_DIR}\qtnetwork4.dll
  File /nonfatal ${QT_DLL_DIR}\mingwm10.dll
!endif


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
  CreateShortCut "$SMPROGRAMS\MikSYS\MKStudio\MKServer.lnk" "$INSTDIR\MKStudio.exe" "" "$INSTDIR\MKServer.exe" 0
SectionEnd

; Optional section (can be disabled by the user)
Section "����� �������� ��� �� Mikkon"
  SetOutPath $INSTDIR\mikkon
  File ..\..\bin\mikkon\*.xml
SectionEnd

Section "������ �� ������� ����"
  CreateShortCut "$DESKTOP\MKStudio.lnk" "$INSTDIR\MKStudio.exe" "" "$INSTDIR\MKStudio.exe" 0
  CreateShortCut "$DESKTOP\MKView.lnk"   "$INSTDIR\MKView.exe"   "" "$INSTDIR\MKView.exe"   0
  CreateShortCut "$DESKTOP\MKServer.lnk" "$INSTDIR\MKServer.exe" "" "$INSTDIR\MKServer.exe" 0
SectionEnd

;----------------------------------------------------------------------------------------------------------------

; Uninstaller

Section "Uninstall"
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\MKStudio"
  DeleteRegKey HKLM SOFTWARE\MKStudio

  ; Remove files and uninstaller
  Delete $INSTDIR\mikkon\*.xml

  Delete $INSTDIR\MKView.exe
  Delete $INSTDIR\MKStudio.exe
  Delete $INSTDIR\MKServer.exe
  Delete $INSTDIR\mkserverd.exe
  Delete $INSTDIR\mkquery.exe
  Delete $INSTDIR\mksync.exe
  Delete $INSTDIR\uninstall.exe

!ifdef QT_DLL_DIR
  Delete $INSTDIR\qtcore4.dll
  Delete $INSTDIR\qtgui4.dll
  Delete $INSTDIR\qtxml4.dll
  Delete $INSTDIR\qtnetwork4.dll
  Delete $INSTDIR\mingwm10.dll
!endif

  Delete $INSTDIR\mkstudio.ini
  Delete $INSTDIR\mkview.ini
  Delete $INSTDIR\mkserver.ini
  Delete $INSTDIR\classic_registers.xml

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\MikSYS\MKStudio\*.*"
  Delete "$DESKTOP\MKStudio.lnk"
  Delete "$DESKTOP\MKView.lnk"
  Delete "$DESKTOP\MKServer.lnk"

  ; Remove directories used
  RMDir "$SMPROGRAMS\MikSYS\MKStudio"
  RMDir "$INSTDIR\mikkon"
  RMDir "$INSTDIR"

SectionEnd
