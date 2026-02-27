<#
.SYNOPSIS
    Serial-Log Capture für InspectAir Tests
.DESCRIPTION
    Zeichnet den Serial-Output des ESP32 mit Zeitstempel auf.
    Erstellt automatisch ein Testrun-Verzeichnis mit Datum.
.PARAMETER ComPort
    COM-Port des ESP32 (z.B. COM5)
.PARAMETER BaudRate
    Baudrate (Standard: 115200)
.PARAMETER Duration
    Aufnahmedauer in Minuten (0 = unbegrenzt, Standard: 0)
.EXAMPLE
    .\capture_serial.ps1 -ComPort COM5
    .\capture_serial.ps1 -ComPort COM5 -Duration 1440    # 24h Dauertest
    .\capture_serial.ps1 -ComPort COM5 -BaudRate 115200 -Duration 30
#>

param(
    [Parameter(Mandatory=$true)]
    [string]$ComPort,
    
    [int]$BaudRate = 115200,
    
    [int]$Duration = 0,

    [string]$OutputDir = ""
)

# ── Testrun-Verzeichnis erstellen ──
$timestamp = Get-Date -Format "yyyy-MM-dd_HHmmss"

if ($OutputDir -eq "") {
    $scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
    $OutputDir = Join-Path $scriptDir "runs\$timestamp"
}

New-Item -ItemType Directory -Path $OutputDir -Force | Out-Null
New-Item -ItemType Directory -Path (Join-Path $OutputDir "photos") -Force | Out-Null

$logFile = Join-Path $OutputDir "serial_log.txt"

Write-Host ""
Write-Host "╔══════════════════════════════════════════════════╗" -ForegroundColor Cyan
Write-Host "║       INSPECTAIR — Serial Log Capture            ║" -ForegroundColor Cyan
Write-Host "╚══════════════════════════════════════════════════╝" -ForegroundColor Cyan
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

# ── Header in Log-Datei ──
$header = @"
# ══════════════════════════════════════════════════════════════
# INSPECTAIR Serial Log
# Datum:    $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")
# Port:     $ComPort @ $BaudRate Baud
# Dauer:    $(if ($Duration -gt 0) { "$Duration Min" } else { "Unbegrenzt" })
# Firmware: $(if (Test-Path (Join-Path (Split-Path $scriptDir -Parent) "..\.git")) { git -C (Join-Path $scriptDir "..\..") rev-parse --short HEAD 2>$null } else { "unbekannt" })
# ══════════════════════════════════════════════════════════════

"@
$header | Out-File -FilePath $logFile -Encoding utf8

# ── Serial Port öffnen ──
try {
    $port = New-Object System.IO.Ports.SerialPort $ComPort, $BaudRate, "None", 8, "One"
    $port.ReadTimeout = 1000
    $port.DtrEnable = $false   # Kein Reset beim Verbinden
    $port.RtsEnable = $false
    $port.Open()
    Write-Host "  [OK] Port $ComPort geöffnet" -ForegroundColor Green
} catch {
    Write-Host "  [FEHLER] Kann $ComPort nicht öffnen: $_" -ForegroundColor Red
    Write-Host ""
    Write-Host "  Verfügbare Ports:" -ForegroundColor Yellow
    [System.IO.Ports.SerialPort]::GetPortNames() | ForEach-Object { Write-Host "    $_" }
    exit 1
}

# ── Statistik ──
$lineCount = 0
$errorCount = 0
$warnCount = 0
$startTime = Get-Date
$endTime = if ($Duration -gt 0) { $startTime.AddMinutes($Duration) } else { [DateTime]::MaxValue }

Write-Host ""
Write-Host "  Aufzeichnung läuft... (Ctrl+C zum Stoppen)" -ForegroundColor Green
Write-Host "  ──────────────────────────────────────────" -ForegroundColor DarkGray
Write-Host ""

# ── Capture-Loop ──
try {
    while ((Get-Date) -lt $endTime) {
        try {
            $line = $port.ReadLine()
            $ts = Get-Date -Format "HH:mm:ss.fff"
            $logLine = "[$ts] $line"
            
            # In Datei schreiben
            $logLine | Out-File -FilePath $logFile -Append -Encoding utf8
            
            # Auf Konsole ausgeben (farbig)
            if ($line -match "\[E\]") {
                Write-Host $logLine -ForegroundColor Red
                $errorCount++
            } elseif ($line -match "\[W\]") {
                Write-Host $logLine -ForegroundColor Yellow
                $warnCount++
            } elseif ($line -match "HEALTH|REPORT|═══") {
                Write-Host $logLine -ForegroundColor Cyan
            } else {
                Write-Host $logLine
            }
            
            $lineCount++
            
            # Alle 100 Zeilen Statistik in Fenstertitel
            if ($lineCount % 100 -eq 0) {
                $elapsed = (Get-Date) - $startTime
                $Host.UI.RawUI.WindowTitle = "InspectAir Log | $lineCount Zeilen | $errorCount Fehler | $([math]::Floor($elapsed.TotalMinutes)) Min"
            }
            
        } catch [System.TimeoutException] {
            # Timeout ist normal, einfach weiter
            continue
        }
    }
} catch {
    # Ctrl+C oder anderer Abbruch
} finally {
    # ── Zusammenfassung ──
    $elapsed = (Get-Date) - $startTime
    
    $summary = @"

# ══════════════════════════════════════════════════════════════
# AUFNAHME BEENDET
# Ende:        $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")
# Dauer:       $([math]::Floor($elapsed.TotalHours))h $($elapsed.Minutes)m $($elapsed.Seconds)s
# Zeilen:      $lineCount
# Fehler [E]:  $errorCount
# Warnungen:   $warnCount
# ══════════════════════════════════════════════════════════════
"@
    $summary | Out-File -FilePath $logFile -Append -Encoding utf8
    
    if ($port.IsOpen) { $port.Close() }
    
    Write-Host ""
    Write-Host "  ──────────────────────────────────────────" -ForegroundColor DarkGray
    Write-Host ""
    Write-Host "  Aufnahme beendet!" -ForegroundColor Green
    Write-Host "  Dauer:       $([math]::Floor($elapsed.TotalHours))h $($elapsed.Minutes)m $($elapsed.Seconds)s" -ForegroundColor Yellow
    Write-Host "  Zeilen:      $lineCount" -ForegroundColor Yellow
    Write-Host "  Fehler [E]:  $errorCount" -ForegroundColor $(if ($errorCount -eq 0) { "Green" } else { "Red" })
    Write-Host "  Warnungen:   $warnCount" -ForegroundColor $(if ($warnCount -eq 0) { "Green" } else { "Yellow" })
    Write-Host "  Log-Datei:   $logFile" -ForegroundColor Yellow
    Write-Host ""
    
    $Host.UI.RawUI.WindowTitle = "PowerShell"
}
