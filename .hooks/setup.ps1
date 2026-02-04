$scriptPath = $PSScriptRoot
Copy-Item "$scriptPath\commit-msg" -Destination "$scriptPath\..\.git\hooks\"
ICACLS "$scriptPath\..\.git\hooks\commit-msg" /grant:r "$env:USERNAME`:(RX)" /C
