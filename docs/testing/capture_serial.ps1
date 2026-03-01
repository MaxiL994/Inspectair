<#
.SYNOPSIS
    Serial-Log Capture for InspectAir Tests
.DESCRIPTION
    Records ESP32 serial output with timestamps.
    Creates a testrun directory with date automatically.
.PARAMETER ComPort
    COM port of ESP32 (e.g. COM5)
.PARAMETER BaudRate
    Baud rate (default: 115200)
.PARAMETER Duration
    Recording duration in minutes (0 = unlimited, default: 0)
.EXAMPLE
    .\capture_serial.ps1 -ComPort COM5
    .\capture_serial.ps1 -ComPort COM5 -Duration 720
#>

param(
    [Parameter(Mandatory=$true)]
    [string]$ComPort,
    
    [int]$BaudRate = 115200,
    
    [int]$Duration = 0,

    [string]$OutputDir = ""
)

# Create testrun directory
$timestamp = Get-Date -Format "yyyy-MM-dd_HHmmss"

if ($OutputDir -eq "") {
    $scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
    $OutputDir = Join-Path $scriptDir "runs\$timestamp"
}

New-Item -ItemType Directory -Path $OutputDir -Force | Out-Null
New-Item -ItemType Directory -Path (Join-Path $OutputDir "photos") -Force | Out-Null

$logFile = Join-Path $OutputDir "serial_log.txt"

Write-Host ""
Write-Host "========================================================" -ForegroundColor Cyan
Write-Host "       INSPECTAIR - Serial Log Capture                  " -ForegroundColor Cyan
Write-Host "========================================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "  COM-Port:    $ComPort" -ForegroundColor Yellow
Write-Host "  Baudrate:    $BaudRate" -ForegroundColor Yellow
Write-Host "  Log-Datei:   $logFile" -ForegroundColor Yellow
if ($Duration -gt 0) {
    Write-Host "  Dauer:       $Duration Minuten" -ForegroundColor Yellow
} else {
    Write-Host "  Dauer:       Unbegrenzt (Ctrl+C zum Stoppen)" -ForegroundColor Yellow
}
Write-Host ""

# Header in log file
$header = @"
# ================================================================
# INSPECTAIR Serial Log
# Datum:    $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")
# Port:     $ComPort @ $BaudRate Baud
# Dauer:    $(if ($Duration -gt 0) { "$Duration Min" } else { "Unbegrenzt" })
# ================================================================

"@
$header | Out-File -FilePath $logFile -Encoding utf8

# Open serial port
try {
    $port = New-Object System.IO.Ports.SerialPort $ComPort, $BaudRate, "None", 8, "One"
    $port.ReadTimeout = 1000
    $port.DtrEnable = $false
    $port.RtsEnable = $false
    $port.Open()
    Write-Host "  [OK] Port $ComPort geoeffnet" -ForegroundColor Green
} catch {
    Write-Host "  [FEHLER] Kann $ComPort nicht oeffnen: $_" -ForegroundColor Red
    Write-Host ""
    Write-Host "  Verfuegbare Ports:" -ForegroundColor Yellow
    [System.IO.Ports.SerialPort]::GetPortNames() | ForEach-Object { Write-Host "    $_" }
    exit 1
}

# Statistics
$lineCount = 0
$errorCount = 0
$warnCount = 0
$startTime = Get-Date
$endTime = if ($Duration -gt 0) { $startTime.AddMinutes($Duration) } else { [DateTime]::MaxValue }

Write-Host ""
Write-Host "  Aufzeichnung laeuft... (Ctrl+C zum Stoppen)" -ForegroundColor Green
Write-Host "  --------------------------------------------------------" -ForegroundColor DarkGray
Write-Host ""

# Capture loop
try {
    while ((Get-Date) -lt $endTime) {
        try {
            $line = $port.ReadLine()
            $ts = Get-Date -Format "HH:mm:ss.fff"
            $logLine = "[$ts] $line"
            
            # Write to file
            $logLine | Out-File -FilePath $logFile -Append -Encoding utf8
            
            # Console output (colored)
            if ($line -match "\[E\]") {
                Write-Host $logLine -ForegroundColor Red
                $errorCount++
            } elseif ($line -match "\[W\]") {
                Write-Host $logLine -ForegroundColor Yellow
                $warnCount++
            } elseif ($line -match "HEALTH|REPORT|===") {
                Write-Host $logLine -ForegroundColor Cyan
            } else {
                Write-Host $logLine
            }
            
            $lineCount++
            
            # Update window title every 100 lines
            if ($lineCount % 100 -eq 0) {
                $elapsed = (Get-Date) - $startTime
                $mins = [math]::Floor($elapsed.TotalMinutes)
                $Host.UI.RawUI.WindowTitle = "InspectAir Log - $lineCount lines - $errorCount errors - $mins min"
            }
            
        } catch [System.TimeoutException] {
            # Timeout is normal, continue
            continue
        }
    }
} catch {
    # Ctrl+C or other abort
} finally {
    # Summary
    $elapsed = (Get-Date) - $startTime
    $hours = [math]::Floor($elapsed.TotalHours)
    $mins = $elapsed.Minutes
    $secs = $elapsed.Seconds
    
    $summary = @"

# ================================================================
# AUFNAHME BEENDET
# Ende:        $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")
# Dauer:       ${hours}h ${mins}m ${secs}s
# Zeilen:      $lineCount
# Fehler [E]:  $errorCount
# Warnungen:   $warnCount
# ================================================================
"@
    $summary | Out-File -FilePath $logFile -Append -Encoding utf8
    
    if ($port.IsOpen) { $port.Close() }
    
    Write-Host ""
    Write-Host "  --------------------------------------------------------" -ForegroundColor DarkGray
    Write-Host ""
    Write-Host "  Aufnahme beendet!" -ForegroundColor Green
    Write-Host "  Dauer:       ${hours}h ${mins}m ${secs}s" -ForegroundColor Yellow
    Write-Host "  Zeilen:      $lineCount" -ForegroundColor Yellow
    if ($errorCount -eq 0) {
        Write-Host "  Fehler [E]:  $errorCount" -ForegroundColor Green
    } else {
        Write-Host "  Fehler [E]:  $errorCount" -ForegroundColor Red
    }
    if ($warnCount -eq 0) {
        Write-Host "  Warnungen:   $warnCount" -ForegroundColor Green
    } else {
        Write-Host "  Warnungen:   $warnCount" -ForegroundColor Yellow
    }
    Write-Host "  Log-Datei:   $logFile" -ForegroundColor Yellow
    Write-Host ""
    
    $Host.UI.RawUI.WindowTitle = "PowerShell"
}
