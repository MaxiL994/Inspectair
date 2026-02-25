<#
.SYNOPSIS
    Automatischer Health-Report Capture für 24h-Dauertest
.DESCRIPTION
    Ruft periodisch /api/health vom ESP32 ab und speichert die JSON-Reports.
    Ideal für den 24h-Dauertest: Stündliche Snapshots + Zusammenfassung am Ende.
.PARAMETER EspIp
    IP-Adresse des ESP32 (z.B. 192.168.1.100)
.PARAMETER IntervalMinutes
    Abruf-Intervall in Minuten (Standard: 60)
.PARAMETER Duration
    Gesamtdauer in Stunden (Standard: 24)
.EXAMPLE
    .\capture_health.ps1 -EspIp 192.168.1.100
    .\capture_health.ps1 -EspIp 192.168.1.100 -IntervalMinutes 30 -Duration 24
#>

param(
    [Parameter(Mandatory=$true)]
    [string]$EspIp,
    
    [int]$IntervalMinutes = 60,
    
    [int]$Duration = 24,

    [string]$OutputDir = ""
)

$timestamp = Get-Date -Format "yyyy-MM-dd_HHmmss"

if ($OutputDir -eq "") {
    $scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
    $OutputDir = Join-Path $scriptDir "runs\$timestamp\health_hourly"
}

New-Item -ItemType Directory -Path $OutputDir -Force | Out-Null

$url = "http://$EspIp/api/health"

Write-Host ""
Write-Host "╔══════════════════════════════════════════════════╗" -ForegroundColor Cyan
Write-Host "║       INSPECTAIR — Health Report Capture         ║" -ForegroundColor Cyan
Write-Host "╚══════════════════════════════════════════════════╝" -ForegroundColor Cyan
Write-Host ""
Write-Host "  ESP32-IP:    $EspIp" -ForegroundColor Yellow
Write-Host "  URL:         $url" -ForegroundColor Yellow
Write-Host "  Intervall:   alle $IntervalMinutes Min" -ForegroundColor Yellow
Write-Host "  Dauer:       $Duration h" -ForegroundColor Yellow
Write-Host "  Ausgabe:     $OutputDir" -ForegroundColor Yellow
Write-Host ""

$startTime = Get-Date
$endTime = $startTime.AddHours($Duration)
$snapCount = 0
$failCount = 0

# ── Zusammenfassungs-CSV vorbereiten ──
$csvFile = Join-Path (Split-Path $OutputDir -Parent) "health_summary.csv"
"Timestamp,Uptime_s,Heap_KB,HeapMin_KB,LoopAvg_ms,LoopMax_ms,SensorErrors,WifiDisconnects,Anomalies,Rating" | Out-File -FilePath $csvFile -Encoding utf8

Write-Host "  Capture läuft... (Ctrl+C zum Stoppen)" -ForegroundColor Green
Write-Host ""

try {
    while ((Get-Date) -lt $endTime) {
        $ts = Get-Date -Format "yyyy-MM-dd_HH-mm-ss"
        $elapsed = (Get-Date) - $startTime
        $hours = [math]::Floor($elapsed.TotalHours)
        $mins = $elapsed.Minutes
        
        try {
            $response = Invoke-RestMethod -Uri $url -TimeoutSec 10
            $json = $response | ConvertTo-Json -Depth 5
            
            # JSON-Snapshot speichern
            $snapFile = Join-Path $OutputDir "health_${ts}.json"
            $json | Out-File -FilePath $snapFile -Encoding utf8
            $snapCount++
            
            # Werte extrahieren und in CSV
            $uptime = if ($response.uptime_s) { $response.uptime_s } else { "?" }
            $heap = if ($response.heap_free_kb) { $response.heap_free_kb } elseif ($response.heap_free) { [math]::Round($response.heap_free / 1024, 1) } else { "?" }
            $heapMin = if ($response.heap_min_kb) { $response.heap_min_kb } elseif ($response.heap_min) { [math]::Round($response.heap_min / 1024, 1) } else { "?" }
            $loopAvg = if ($response.loop_avg_ms) { $response.loop_avg_ms } else { "?" }
            $loopMax = if ($response.loop_max_ms) { $response.loop_max_ms } else { "?" }
            $sensorErr = if ($response.sensor_errors_total -ne $null) { $response.sensor_errors_total } else { "?" }
            $wifiDisc = if ($response.wifi_disconnects -ne $null) { $response.wifi_disconnects } else { "?" }
            $anomalies = if ($response.anomaly_count -ne $null) { $response.anomaly_count } else { "?" }
            $rating = if ($response.health_rating) { $response.health_rating } else { "?" }
            
            "$ts,$uptime,$heap,$heapMin,$loopAvg,$loopMax,$sensorErr,$wifiDisc,$anomalies,$rating" | Out-File -FilePath $csvFile -Append -Encoding utf8
            
            # Konsolenausgabe
            $color = switch ($rating) {
                "PERFEKT"    { "Green" }
                "GUT"        { "Cyan" }
                "AKZEPTABEL" { "Yellow" }
                "KRITISCH"   { "Red" }
                default      { "White" }
            }
            Write-Host "  [${hours}h${mins}m] Snap #$snapCount | Heap: ${heap}KB | Errors: $sensorErr | Rating: $rating" -ForegroundColor $color
            
        } catch {
            $failCount++
            Write-Host "  [${hours}h${mins}m] FEHLER: $($_.Exception.Message)" -ForegroundColor Red
            "$ts,FEHLER,,,,,,,,ESP nicht erreichbar" | Out-File -FilePath $csvFile -Append -Encoding utf8
        }
        
        # Warten bis zum nächsten Intervall
        Start-Sleep -Seconds ($IntervalMinutes * 60)
    }
} catch {
    # Ctrl+C
} finally {
    Write-Host ""
    Write-Host "  ──────────────────────────────────────────" -ForegroundColor DarkGray
    Write-Host "  Capture beendet!" -ForegroundColor Green
    Write-Host "  Snapshots:   $snapCount" -ForegroundColor Yellow
    Write-Host "  Fehlschläge: $failCount" -ForegroundColor $(if ($failCount -eq 0) { "Green" } else { "Red" })
    Write-Host "  CSV:         $csvFile" -ForegroundColor Yellow
    Write-Host "  Snapshots:   $OutputDir" -ForegroundColor Yellow
    Write-Host ""
}
