; The Windows installer (Inno Setup 6): YeetDownloader-<version>-setup-x64.exe.
; Built in CI from the app folder that windeployqt filled:
;   iscc /DAppVersion=2.0 /DSourceDir=<full path of dist\YeetDownloader> /DOutputDir=<full path> release\windows\YeetDownloader.iss
; Installs for the current user (no administrator prompt) into
; %LOCALAPPDATA%\Programs\YeetDownloader. If ffmpeg is missing it offers to
; install it with winget. yt-dlp is fetched and checked by the app itself.

#ifndef AppVersion
  #define AppVersion "0.0.0"
#endif
#ifndef SourceDir
  #define SourceDir "..\..\dist\YeetDownloader"
#endif
#ifndef OutputDir
  #define OutputDir "..\..\installer"
#endif

[Setup]
; (The id Windows knows the installed app by: never change it)
AppId={{8F3B6C2E-5A4D-4E7B-9C1A-2D6E8F0B3A57}
AppName=YeetDownloader
AppVersion={#AppVersion}
AppVerName=YeetDownloader {#AppVersion}
AppPublisher=Chandan Das
AppPublisherURL=https://github.com/ChandanDasGIT/YeetDownloader
AppSupportURL=https://github.com/ChandanDasGIT/YeetDownloader/issues
DefaultDirName={autopf}\YeetDownloader
DefaultGroupName=YeetDownloader
DisableProgramGroupPage=yes
PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
MinVersion=10.0.17763
SetupIconFile=YeetDownloader.ico
UninstallDisplayIcon={app}\YeetDownloader.exe
UninstallDisplayName=YeetDownloader
OutputDir={#OutputDir}
OutputBaseFilename=YeetDownloader-{#AppVersion}-setup-x64
Compression=lzma2/max
SolidCompression=yes
WizardStyle=modern
CloseApplications=yes
RestartApplications=yes

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "ffmpeg"; Description: "Install ffmpeg with winget (needed to make MP3 and MP4 files)"; GroupDescription: "Dependencies:"; Check: not FfmpegFound

[Files]
Source: "{#SourceDir}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{autoprograms}\YeetDownloader"; Filename: "{app}\YeetDownloader.exe"
Name: "{autodesktop}\YeetDownloader"; Filename: "{app}\YeetDownloader.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\YeetDownloader.exe"; Description: "{cm:LaunchProgram,YeetDownloader}"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
; The folder itself, once empty (yt-dlp and the settings live elsewhere)
Type: dirifempty; Name: "{app}"

[Code]
function FfmpegFound: Boolean;
begin
  Result := (FileSearch('ffmpeg.exe', GetEnv('PATH')) <> '')
    or FileExists(ExpandConstant('{localappdata}\Microsoft\WinGet\Links\ffmpeg.exe'))
    or FileExists(ExpandConstant('{commonpf64}\WinGet\Links\ffmpeg.exe'));
end;

procedure InstallFfmpeg;
var
  Winget: String;
  ResultCode: Integer;
begin
  Winget := ExpandConstant('{localappdata}\Microsoft\WindowsApps\winget.exe');
  if not FileExists(Winget) then
  begin
    SuppressibleMsgBox('winget (Windows'' package manager) was not found, so ffmpeg could not be installed.' + #13#10#13#10 +
      'YeetDownloader needs ffmpeg to make MP3 and MP4 files. Install "App Installer" from the Microsoft Store, ' +
      'then run in a terminal:' + #13#10 + '    winget install Gyan.FFmpeg', mbInformation, MB_OK, IDOK);
    Exit;
  end;
  WizardForm.StatusLabel.Caption := 'Installing ffmpeg with winget (this can take a minute)...';
  if not Exec(Winget, 'install --id Gyan.FFmpeg --exact --scope user --silent --accept-source-agreements --accept-package-agreements',
              '', SW_HIDE, ewWaitUntilTerminated, ResultCode) or (ResultCode <> 0) then
    SuppressibleMsgBox('ffmpeg could not be installed (winget exit code ' + IntToStr(ResultCode) + ').' + #13#10#13#10 +
      'YeetDownloader needs ffmpeg to make MP3 and MP4 files. Run in a terminal:' + #13#10 +
      '    winget install Gyan.FFmpeg', mbInformation, MB_OK, IDOK);
end;

procedure CurStepChanged(CurStep: TSetupStep);
begin
  if (CurStep = ssPostInstall) and WizardIsTaskSelected('ffmpeg') then
    InstallFfmpeg;
end;
