; example1.nsi
;
; This script is perhaps one of the simplest NSIs you can make. All of the
; optional settings are left to their default settings. The installer simply 
; prompts the user asking them where to install, and drops a copy of example1.nsi
; there. 

;--------------------------------

; The name of the installer
Name "breveCreatures"

; The file to write
OutFile "breveCreatures.exe"

; The default installation directory
InstallDir $WINDIR

LicenseData "License.rtf"

;--------------------------------

; Pages

; Page directory
Page license
Page instfiles

;--------------------------------

; The stuff to install
Section "" ;No components page, name is not important

  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put file there
  File /r breve  
  File breve.scr
  File glut32.dll
  
SectionEnd ; end the section
