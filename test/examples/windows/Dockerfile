FROM mcr.microsoft.com/powershell:windowsservercore-ltsc2022

SHELL ["pwsh", "-Command", "$ErrorActionPreference = 'Stop'; $ProgressPreference = 'SilentlyContinue';"]

# Install Chocolately
RUN Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))

# Install VC++ Redistributable 2022
RUN choco install -y vcredist140

# Install OpenSSL latest
RUN choco install -y openssl

#Install Runtime Debug Libraries
RUN choco install -y visualstudio2022buildtools --package-parameters '--includeRecommended --includeOptional --add Microsoft.VisualStudio.Component.VC.Tools.x86.x64 --add Microsoft.Component.VC.Runtime.UCRTSDK' --no-progress
