; Instalador Windows (.exe) da AURA — copia o VST3 automaticamente para
; a pasta comum do VST3 e a Standalone para Program Files, para o
; utilizador não ter de saber/copiar nada manualmente (ao contrário do
; .zip simples, que exige colocar o VST3 na pasta certa à mão — a causa
; mais provável de um plugin "não aparecer" nalguns DAWs).
;
; Espera que o passo de CI já tenha preparado uma pasta "dist" ao lado
; deste ficheiro (dist\Premoli Labs AURA.vst3\... e
; dist\Premoli Labs AURA.exe), tal como a usada para montar o .zip.

#define MyAppName "Premoli Labs AURA"
#define MyShortName "AURA"
#define MyPublisher "Premoli Labs"

[Setup]
AppName={#MyAppName}
AppVersion=1.0
AppPublisher={#MyPublisher}
DefaultDirName={autopf}\{#MyPublisher}\{#MyShortName}
DisableProgramGroupPage=yes
OutputDir=.
OutputBaseFilename=PremoliLabs-{#MyShortName}-Installer
Compression=lzma
SolidCompression=yes
ArchitecturesInstallIn64BitMode=x64compatible
UninstallFilesDir={autopf}\{#MyPublisher}\{#MyShortName}\Uninstall
WizardStyle=modern

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
; Agrupado numa subpasta "Premoli Labs" dentro da pasta comum do VST3,
; para todos os plug-ins da marca aparecerem juntos no navegador de
; plug-ins do DAW em vez de espalhados individualmente (mesma
; convenção usada pela Waves, FabFilter, etc.).
Source: "..\dist\{#MyAppName}.vst3\*"; DestDir: "{commoncf64}\VST3\{#MyPublisher}\{#MyAppName}.vst3"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\dist\{#MyAppName}.exe"; DestDir: "{autopf}\{#MyPublisher}\{#MyShortName}"; Flags: ignoreversion

[Icons]
Name: "{autoprograms}\{#MyPublisher}\{#MyAppName}"; Filename: "{autopf}\{#MyPublisher}\{#MyShortName}\{#MyAppName}.exe"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{autopf}\{#MyPublisher}\{#MyShortName}\{#MyAppName}.exe"; Tasks: desktopicon
