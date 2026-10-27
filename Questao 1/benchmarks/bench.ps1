param(
    [string]$Platform = "maquina",
    [UInt64]$Terms = 1000000000,
    [int]$Repetitions = 5,
    [int[]]$ThreadCounts = @(1, 2, 4, 8, 16, 32)
)

$ErrorActionPreference = "Stop"

if ($Platform -notmatch '^[A-Za-z0-9_-]+$') {
    throw "Use um nome de plataforma contendo apenas letras, numeros, _ ou -."
}
if ($Terms -lt 1 -or $Repetitions -lt 1) {
    throw "Terms e Repetitions devem ser positivos."
}
if ($ThreadCounts.Count -eq 0 -or $ThreadCounts | Where-Object { $_ -lt 1 -or $_ -gt 256 }) {
    throw "ThreadCounts deve conter valores entre 1 e 256."
}

$source = Join-Path $PSScriptRoot "pi_threads.c"
$executable = Join-Path $PSScriptRoot "pi_threads.exe"
$output = Join-Path (Get-Location) "tempos_$Platform.csv"

$gcc = Get-Command gcc -ErrorAction SilentlyContinue
if (-not $gcc) {
    throw "gcc nao encontrado. Abra o terminal MSYS2 UCRT64 e instale mingw-w64-ucrt-x86_64-gcc."
}

$msysRoot = Split-Path (Split-Path (Split-Path $gcc.Source -Parent) -Parent) -Parent
$bash = Join-Path $msysRoot "usr\bin\bash.exe"
if (-not (Test-Path $bash)) {
    throw "Nao encontrei o bash do MSYS2 em $bash."
}

$msysProject = $PSScriptRoot -replace '\\', '/'
if ($msysProject -match '^([A-Za-z]):(.*)$') {
    $msysProject = "/$($Matches[1].ToLower())$($Matches[2])"
}
$compileCommand = "export PATH=/ucrt64/bin:/usr/bin:`$PATH; cd `"$msysProject`" && gcc -O2 -Wall -Wextra -o pi_threads.exe pi_threads.c -lm -lpthread"
& $bash -lc $compileCommand
if ($LASTEXITCODE -ne 0) {
    throw "Falha ao compilar pi_threads.c."
}

$encoding = [System.Text.UTF8Encoding]::new($false)
$writer = [System.IO.StreamWriter]::new($output, $false, $encoding)
try {
    $writer.WriteLine("plataforma,threads,execucao,tempo_s")
    foreach ($threads in $ThreadCounts) {
        for ($run = 1; $run -le $Repetitions; $run++) {
            $process = [System.Diagnostics.Process]::new()
            $process.StartInfo.FileName = $executable
            $process.StartInfo.Arguments = "$threads $Terms"
            $process.StartInfo.UseShellExecute = $false
            $process.StartInfo.CreateNoWindow = $true
            $process.StartInfo.RedirectStandardOutput = $true
            $process.StartInfo.RedirectStandardError = $true

            $timer = [System.Diagnostics.Stopwatch]::StartNew()
            [void]$process.Start()
            $stdout = $process.StandardOutput.ReadToEnd()
            $stderr = $process.StandardError.ReadToEnd()
            $process.WaitForExit()
            $timer.Stop()

            if ($process.ExitCode -ne 0) {
                throw "Falha na execucao com $threads threads: $stderr"
            }

            $seconds = $timer.Elapsed.TotalSeconds.ToString("F6", [Globalization.CultureInfo]::InvariantCulture)
            $writer.WriteLine(("{0},{1},{2},{3}" -f $Platform, $threads, $run, $seconds))
            Write-Host "threads=$threads execucao=$run tempo=${seconds}s $($stdout.Trim())"
            $process.Dispose()
        }
    }
}
finally {
    $writer.Dispose()
}

Write-Host "Resultados em $output"