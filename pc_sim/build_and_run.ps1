param(
    [string]$Compiler = "auto"
)

$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $PSScriptRoot
$BuildDir = Join-Path $PSScriptRoot "build"
$Exe = Join-Path $BuildDir "securevoice_pc_sim.exe"

New-Item -ItemType Directory -Force -Path $BuildDir | Out-Null

function Find-Command($Name) {
    $cmd = Get-Command $Name -ErrorAction SilentlyContinue
    if ($cmd) {
        return $cmd.Source
    }
    return $null
}

$gcc = Find-Command "gcc"
$clang = Find-Command "clang"

if ($Compiler -eq "auto") {
    if ($gcc) {
        $Compiler = "gcc"
    } elseif ($clang) {
        $Compiler = "clang"
    } else {
        throw "No supported C compiler was found in PATH. Install MinGW-w64 GCC or LLVM Clang, then run this script again."
    }
}

if ($Compiler -eq "gcc") {
    & gcc -std=gnu89 -Wall -Wextra -Wno-implicit-function-declaration `
        -I (Join-Path $Root "inc") `
        -o $Exe `
        (Join-Path $Root "src\main.c") `
        (Join-Path $PSScriptRoot "pc_stubs.c")
} elseif ($Compiler -eq "clang") {
    & clang -std=gnu89 -Wall -Wextra -Wno-implicit-function-declaration `
        -I (Join-Path $Root "inc") `
        -o $Exe `
        (Join-Path $Root "src\main.c") `
        (Join-Path $PSScriptRoot "pc_stubs.c")
} else {
    throw "Unsupported compiler '$Compiler'. Use auto, gcc, or clang."
}

& $Exe
