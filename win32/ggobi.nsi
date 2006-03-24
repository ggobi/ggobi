!include "AddToPath.NSH"
!include "MUI.nsh"

Name GGobi
VIProductVersion ${VERSION}
OutFile ggobi-${VERSION}.exe
InstallDir $PROGRAMFILES\ggobi
InstallDirRegKey HKLM "Software\ggobi" "InstallationDirectory"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "../CPLicense.txt"
#!insertmacro MUI_PAGE_COMPONENTS components
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

Section "GGobi"
	SetOutPath $INSTDIR
	File ..\src\.libs\ggobi.exe
	File /r /x .svn /x big* ..\data
	#File /r /x .svn ..\doc
	File ..\share\ggobi.ico
	File ..\ggobirc
	File ..\src\.libs\libggobi-0.dll
	
	SetOutPath $INSTDIR\share
	File ..\share\colorschemes.xml
	
	# Install the built plugins
	SetOutPath $INSTDIR\plugins\DataViewer
	File ..\plugins\DataViewer\plugin.la
	File ..\plugins\DataViewer\.libs\plugin-0.dll
	SetOutPath $INSTDIR\plugins\ggvis
	File ..\plugins\ggvis\plugin.la
	File ..\plugins\ggvis\.libs\plugin-0.dll
	SetOutPath $INSTDIR\plugins\VarCloud
	File ..\plugins\VarCloud\plugin.la
	File ..\plugins\VarCloud\.libs\plugin-0.dll
	SetOutPath $INSTDIR\plugins\DescribeDisplay
	File ..\plugins\DescribeDisplay\plugin.la
	File ..\plugins\DescribeDisplay\.libs\plugin-0.dll
	
	CreateShortCut "$DESKTOP\ggobi.lnk" "$INSTDIR\ggobi.exe" "" "$INSTDIR\ggobi.ico"

	Push $INSTDIR
	Call AddToPath
	
	WriteRegStr HKLM SOFTWARE\ggobi "InstallationDirectory" "$INSTDIR"
	
	WriteUninstaller $INSTDIR\uninstaller.exe
SectionEnd
