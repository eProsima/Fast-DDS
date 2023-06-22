# Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# @file add_source_listing.ps1
#
# Preconditions:
#  In order for the algorithm to work with github pull request the associated repos
#  must enable the tracking of the pull request references. This can be set globally in
#  the CI as:
#      git config --global remote.origin.fetch '+refs/pull/*:refs/remotes/origin/pull/*'
#  Note actions/checkout preempts it and may be force locally in the repo as:
#      git config --local remote.origin.fetch '+refs/pull/*:refs/remotes/origin/pull/*'; git fetch origin

Param(
[Parameter(Mandatory=$true,
ParameterSetName='shell',
HelpMessage = 'Pdb files to update with source indexing')]
[ValidateScript({Test-Path $_ -PathType Leaf})]
[String[]]$pdbs,
[Parameter(Mandatory=$true,
ParameterSetName='cli',
Position=0,
HelpMessage = 'Pdb files to update with source indexing as single string using ; as separator. Convenient to call the
script from cmd.')]
[ValidateScript({ $_ -split(";") | % { Test-Path $_ -PathType Leaf }})]
[String]$pdbfiles,
[Parameter(
HelpMessage = 'Path to the srcsrv ancillary tools')]
[ValidateScript({(Test-Path $_ -PathType Container) -and (ls -Path (Join-Path $_ *) -Include pdbstr.exe, srctool.exe) })]
[String]$toolsPath
)

$ErrorActionPreference = 'Stop'

# Turn cli string into array
if($pdbfiles)
{
    $pdbs = $pdbfiles.split(";")
}

if($toolsPath)
{
    $pdbstr = (ls -path $toolsPath -Filter pdbstr.exe).fullname
    $srctool = (ls -path $toolsPath -Filter srctool.exe).fullname
}
else
{
    # if not provided make some introspection
    $kitskey = gi "HKLM:SOFTWARE/Microsoft/Windows Kits/Installed Roots"
    $kitspath = $kitskey.GetValue($kitskey.GetValueNames() -match "KitsRoot")
    $pdbstr = Resolve-Path "$kitspath/*/x64/srcsrv/pdbstr.exe"
    $srctool = Resolve-Path "$kitspath/*/x64/srcsrv/srctool.exe"
}

Write-Verbose "Framework tools: '$pdbstr' '$srctool'"

$ErrorActionPreference = 'SilentlyContinue'

class ExcludePaths {
    [string[]]$dirs

    [bool] Exclude([string]$file) {
        # Change \ to / (avoid regex escaping issues)
        return [bool]$this.dirs.Count -and
               [bool]($file.replace("\","/") | sls -Pattern $this.dirs)
    }

    [void] Add([string]$file) {
       # Precondition: the file is not excluded
       # 1. Get the folder
       $dir = ($file | Split-Path).replace("\","/")
       # 2. Remove those entries that are subfolders of the new one
       if($this.dirs)
       {
           $this.dirs = ($this.dirs | sls -Pattern $dir -NotMatch).Line
       }
       # 3. Add the new one
       $this.dirs += @($dir.replace("(","\(").replace(")","\)"))
    }
}

$excludes = New-Object -TypeName ExcludePaths

class Repositories {
    # Each individual repo keys:
    # - name
    # - commit
    # - path
    # - submodules array
    [PSCustomObject[]]$repos

    [PSCustomObject] GetRepo([string]$file){
        return $this.GetRepo($file, $this.repos)
    }

    [bool] AddRepo([string]$file){
        $new = [PSCustomObject]@{
            name = ""
            commit = ""
            path = ""
            submodules = @()
        }
        $res = $this.AddRepo($file, $new)
        if($res)
        {
            $this.repos += $new
        }
        return $res
    }

    # Recursive implementation methods

    [PSCustomObject] GetRepo([string]$file, [PSCustomObject[]]$col){
        # ignore if empty
        if($col.Count -eq 0) { return $null}
        # Search for a repo whose path matches the file
        $file = $file.replace("\","/")
        $repo = $col.Where({$file -match $_.path},'SkipUntil',1)
        # Check if the $file belong to some submodule
        $subrepo = $Null
        if($repo.submodules) {
            $subrepo = $this.GetRepo($file, $repo.submodules)
        }
        # return $subrepo ?? $repo
        if($subrepo) {
            return $subrepo
        } else {
            return $repo
        }
    }

    [bool] AddRepo([string]$file, [PSCustomObject]$new){
        # Precondition the $file is not associated with a known repo

        # get associated idr
        $dir = $file
        if(Test-Path $file -PathType Leaf) {
            $dir = ($file | Split-Path)
        }
        $dir.replace("\","/")

        $new.commit = git -C $dir log -n1 --pretty=format:'%h' 2>$null

        if($new.commit)
        {
            Write-Verbose "Found new repo commit: git -C $dir log -n1 --pretty=format:'%h' => $($new.commit)"
        }

        if($new.commit -eq $null) { return $false }
        # Find out the repo
        $branches = git -C $dir branch -r --contains $new.commit

        if($branches)
        {
            Write-Verbose "Commit branches: git -C $dir branch -r --contains $($new.commit) => $branches"
        }

        # get associated repos
        $candidates = $branches | sls '^\s*(?<remote>\w+)/' | select -ExpandProperty Matches |
            select -ExpandProperty Groups | ? name -eq 'remote' | sort | unique |
            select -ExpandProperty Value

        # filter out repo list
        $repositories = (git -C $dir remote -v |
            sls "^(?<remote>\w+)\s+https://github.com/((?<repo>\S+)\.git|(?<repo>\S+))").Matches | select -Unique |
            % { [PSCustomObject]@{ repo = [String]$_.Groups['repo']; remote = [String]$_.Groups['remote']}}

        Write-Verbose "Repos available: $($repositories.remote)"

        $new.name = ($repositories | ? remote -in $candidates | Select -First 1).repo

        # on error fallback to origin
        if($new.name)
        {
            Write-Verbose "Chosen repo: $($new.name)"
        }
        else
        {
            if ('origin' -in $repositories.remote)
            {
                $new.name = ($repositories | ? remote -eq 'origin' | Select -First 1).repo
            }
            else
            {
                $new.name = $repositories[0].repo
            }

            Write-Verbose "Cannot track commit: falling back to $($new.name)"
        }

        # get the path
        $new.path = git -C $dir rev-parse --show-toplevel

        # populate submodules
        $matches = (git -C $dir submodule | sls "^\s*\w+ (?<relpath>[\S]+)").Matches
        if($matches)
        {
            $new.submodules = $matches | % {
                    $subdir = "{0}/{1}" -f $new.path, $_.Groups['relpath']
                    $subnew = [PSCustomObject]@{
                        name = ""
                        commit = ""
                        path = ""
                        submodules = @()
                    }
                    if($this.AddRepo($subdir, $subnew)) { $subnew }
                }
        }

        return $true;
    }
}

$repos = New-Object -TypeName Repositories

# Script to process the files into entries
$process = {

    Write-Verbose "Processing file: $_"

    $entry = @{}
    # keep the file as id
    $entry.id = $_

    # Check if the file should be excluded
    if($excludes.Exclude($_))
    {
        Write-Verbose "Excluding file using database: $_"
        return
    }

    # Check the commit
    $repo = $repos.GetRepo($_)

    # If there is no repo try to find one
    if(!$repo)
    {
        if($repos.AddRepo($_))
        {
            # retrieve the repo
            $repo = $repos.GetRepo($_)
        }
        else
        {
            # If there is no repo ignore and update exclude
            $excludes.Add($_)
            Write-Verbose "Excluding file for lack of repo: $_"
            return
        }
    }

    $entry.commit = $repo.commit
    $entry.repo = $repo.name
    $entry.repo_path = $repo.path

    # propagate
    $entry = [PSCustomObject]$entry
    Write-Output $entry
    Write-Verbose "New path entry: $entry"
}

foreach ($pdbfile in $pdbs)
{
    # Extract files and generate entries
    $entries = & $srctool -r $pdbfile | Select -SkipLast 1 | % $process

    # keep the relative path
    $groups = ($entries | ? repo -ne $null) | Group-Object -Property repo
    $groups | % {
        # calculate the relative path for all files
        pushd $_.Group[0].repo_path
        $_.Group | % {
            $rp = (Resolve-Path -Path $_.id -Relative).replace(".\","").replace("\","/")
            Add-Member -InputObject $_ -MemberType NoteProperty `
                       -Name relpath -Value $rp }
        popd
    }

    # Generate the stream
$header = @'
SRCSRV: ini ------------------------------------------------
VERSION=2
VERCTRL=eProsima-Github
DATETIME={0}
SRCSRV: variables ------------------------------------------
SRCSRVTRG=https://raw.githubusercontent.com/%var2%/%var3%/%var4%
SRCSRV: source files ---------------------------------------
'@ -f (Get-Date -Format "ddd, dd MMMM yyyy")

    $header = $header -split "`r?`n"

    $footer = 'SRCSRV: end ------------------------------------------------'

    # $tmp = New-TemporaryFile
    $tmp = Join-Path $Env:TMP (Get-Random)
    $entries | % { $header } {"{0}*{1}*{2}*{3}" -f $_.id, $_.repo, $_.commit, $_.relpath } { $footer } | Out-File $tmp -Encoding OEM

    # incorporate the stream into the file
    & $pdbstr -w "-p:$pdbfile" -s:srcsrv "-i:$tmp"
    Write-Verbose "Stream check: & `"$pdbstr`" -r -p:`"$pdbfile`" -s:srcsrv"
}
