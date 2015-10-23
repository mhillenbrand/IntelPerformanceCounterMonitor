
REM change path to your VCVARS.BAT
CALL "c:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\bin\x86_amd64\vcvarsx86_amd64.bat"

for %%p in (PCM) do (
   @echo Building %%p
   chdir %%p_Win
   msbuild  %%p.vcxproj /p:Configuration=Release  /t:Clean,Build
   chdir ..
)

   @echo Building Intelpcm.dll
   chdir Intelpcm.dll
   msbuild  Intelpcm.dll.vcxproj /p:Configuration=Release  /t:Clean,Build
   chdir ..

   @echo Building PCM-Service 
   chdir PCM-Service_Win
   msbuild  PCMService.vcxproj /p:Configuration=Release  /t:Clean,Build
   chdir ..


for %%p in (PCM-MSR PCM-TSX PCM-Memory PCM-NUMA PCM-PCIE PCM-Power PCM-Core) do (
   @echo Building %%p
   chdir %%p_Win
   msbuild  %%p-win.vcxproj /p:Configuration=Release  /t:Clean,Build
   chdir ..
)

exit


