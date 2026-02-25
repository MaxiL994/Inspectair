# INSPECTAIR Testing

## Dateien

| Datei | Beschreibung |
|-------|-------------|
| `TESTPLAN.md` | Ausführungsschema: Reihenfolge, Abhängigkeiten, Bewertungskriterien |
| `TESTPROTOKOLL.md` | Template — pro Testdurchlauf kopieren und ausfüllen |
| `capture_serial.ps1` | PowerShell: Serial-Log mit Zeitstempel aufzeichnen |
| `capture_health.ps1` | PowerShell: `/api/health` periodisch abrufen (für 24h-Test) |

## Schnellstart

### 1. Serial-Log aufzeichnen
```powershell
.\capture_serial.ps1 -ComPort COM5
```

### 2. Testprotokoll kopieren
```powershell
$run = "runs\$(Get-Date -Format 'yyyy-MM-dd_HHmmss')"
New-Item -ItemType Directory -Path $run -Force
Copy-Item TESTPROTOKOLL.md "$run\TESTPROTOKOLL.md"
```

### 3. Tests durchführen
Protokoll in `runs/<datum>/TESTPROTOKOLL.md` ausfüllen.

### 4. 24h-Dauertest (optional)
```powershell
# Terminal 1: Serial-Log
.\capture_serial.ps1 -ComPort COM5 -Duration 1440

# Terminal 2: Health-Snapshots
.\capture_health.ps1 -EspIp 192.168.1.100 -IntervalMinutes 60 -Duration 24
```

## Testrun-Verzeichnis

Ergebnisse landen in `runs/` (gitignored):
```
runs/
  2026-02-23_143000/
    TESTPROTOKOLL.md      ← Ausgefülltes Protokoll
    serial_log.txt        ← Serial-Mitschnitt mit Timestamps
    health_summary.csv    ← Stündliche Health-Daten
    health_hourly/        ← JSON-Snapshots
    photos/               ← Screen-Fotos
```
