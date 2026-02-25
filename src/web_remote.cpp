/**
 * ═══════════════════════════════════════════════════════════════════════════
 * INSPECTAIR - WEB REMOTE CONTROL
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * HTTP-Webserver für Handy-Fernsteuerung.
 * - Responsive Webseite mit Buttons für Screen-Wechsel
 * - Live-Sensorwerte (Auto-Refresh alle 5s)
 * - 24h Liniendiagramm (CO2, VOC, PM2.5)
 */

#include "web_remote.h"
#include <WebServer.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include "display/ui_manager.h"
#include "utils/power_manager.h"
#include "utils/sensor_filter.h"
#include "utils/sensor_history.h"

static WebServer server(80);

// ============================================
// SCREEN NAMES (müssen zu UIScreen enum passen)
// ============================================
static const char* screenNames[] = {
    "Baum",        // UI_SCREEN_TREE
    "Übersicht",   // UI_SCREEN_OVERVIEW
    "Detail",      // UI_SCREEN_DETAIL
    "Analog",      // UI_SCREEN_ANALOG
    "Bubbles"      // UI_SCREEN_BUBBLE
};

// ============================================
// HTML PAGE mit Auto-Refresh & Chart
// ============================================
static const char HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="de">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>InspectAir Remote</title>
<style>
*{box-sizing:border-box;margin:0;padding:0}
body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',sans-serif;background:#0a0e17;color:#e0e0e0;min-height:100vh;padding:12px}
h1{text-align:center;font-size:1.3em;color:#4fc3f7;margin-bottom:12px}
h2{font-size:1em;color:#81d4fa;margin:14px 0 6px}
.si{text-align:center;padding:6px;background:#1a237e;border-radius:8px;margin-bottom:8px;font-size:1em}
.bg{display:grid;grid-template-columns:1fr 1fr;gap:6px;margin-bottom:10px}
.b{display:block;padding:12px 6px;background:#1e88e5;color:#fff;text-decoration:none;border-radius:10px;text-align:center;font-size:.95em;border:none;cursor:pointer;transition:background .2s}
.b:active{background:#1565c0}
.bn{grid-column:1/-1;background:#00c853;font-size:1.1em}
.bn:active{background:#00a844}
.bp{background:#ff9800}
.sb{display:flex;justify-content:space-between;padding:5px 10px;background:#1a1f2e;border-radius:8px;margin-bottom:6px;font-size:.8em}
.sb .st{color:#4fc3f7;font-weight:bold}
.vs{display:grid;grid-template-columns:1fr 1fr;gap:6px}
.vc{background:#1a1f2e;border-radius:10px;padding:10px;text-align:center}
.vc .l{font-size:.75em;color:#90a4ae}
.vc .v{font-size:1.5em;font-weight:bold;color:#4fc3f7;transition:color .3s}
.vc .u{font-size:.65em;color:#607d8b}
.vc .v.flash{color:#fff}
/* Dimm-Button */
.bdim{background:#ff9800;grid-column:1/-1;font-size:1.05em}
.bdim:active{background:#e68900}
/* Retro Kippschalter */
.sw-wrap{display:flex;align-items:center;justify-content:center;gap:12px;margin:10px 0}
.sw-label{font-size:.85em;color:#90a4ae}
.sw-label.on{color:#4caf50;font-weight:bold}
.sw-label.off{color:#f44336;font-weight:bold}
.flip{position:relative;width:70px;height:120px;cursor:pointer;perspective:300px}
.flip-body{position:absolute;width:100%;height:100%;border-radius:12px;background:linear-gradient(180deg,#3a3a3a 0%,#222 100%);box-shadow:inset 0 2px 4px rgba(255,255,255,.15),0 4px 12px rgba(0,0,0,.6);overflow:hidden;border:2px solid #555}
.flip-track{position:absolute;left:50%;top:8px;bottom:8px;width:6px;margin-left:-3px;background:#111;border-radius:3px;box-shadow:inset 0 1px 3px rgba(0,0,0,.8)}
.flip-knob{position:absolute;left:50%;width:52px;height:40px;margin-left:-26px;border-radius:8px;transition:top .25s cubic-bezier(.4,0,.2,1),background .25s,box-shadow .25s;box-shadow:0 3px 8px rgba(0,0,0,.5),inset 0 1px 2px rgba(255,255,255,.2)}
.flip-knob.on{top:10px;background:linear-gradient(180deg,#66bb6a,#388e3c);box-shadow:0 3px 8px rgba(0,0,0,.5),0 0 16px rgba(76,175,80,.4),inset 0 1px 2px rgba(255,255,255,.3)}
.flip-knob.off{top:68px;background:linear-gradient(180deg,#999,#666);box-shadow:0 3px 8px rgba(0,0,0,.5),inset 0 1px 2px rgba(255,255,255,.15)}
.flip-knob::after{content:'';position:absolute;left:50%;top:50%;width:30px;height:4px;margin:-2px 0 0 -15px;background:rgba(255,255,255,.2);border-radius:2px}
.flip-ind{width:10px;height:10px;border-radius:50%;margin:0 auto;transition:background .3s,box-shadow .3s}
.flip-ind.on{background:#4caf50;box-shadow:0 0 10px #4caf50}
.flip-ind.off{background:#555;box-shadow:none}
.chart-wrap{background:#1a1f2e;border-radius:10px;padding:10px;margin-top:10px}
.chart-wrap canvas{width:100%;height:200px}
.legend{display:flex;justify-content:center;gap:14px;margin-top:6px;font-size:.75em}
.legend span{display:flex;align-items:center;gap:4px}
.legend .dot{width:10px;height:10px;border-radius:50%;display:inline-block}
.rn{text-align:center;margin-top:8px;font-size:.7em;color:#546e7a}
</style>
</head>
<body>
<h1>&#127807; InspectAir</h1>

<div class="si">Aktuell: <strong id="scr">%SCREEN%</strong></div>
<div class="sb">
  <span>Status: <span class="st" id="ds">%DISPSTATE%</span></span>
  <span id="upd" style="color:#4caf50">&#9679; Live</span>
</div>

<a class="b bn" href="/next">&#9654; N&auml;chster Screen</a>

<h2>Screens</h2>
<div class="bg">
  <a class="b" href="/screen?id=0">&#127795; Baum</a>
  <a class="b" href="/screen?id=1">&#128202; &Uuml;bersicht</a>
  <a class="b" href="/screen?id=2">&#128203; Detail</a>
  <a class="b" href="/screen?id=3">&#127902; Analog</a>
  <a class="b" href="/screen?id=4">&#129531; Bubbles</a>
  <a class="b bdim" id="dbtn" href="/dim">%DIMBTNTEXT%</a>
</div>

<h2>Display</h2>
<div class="sw-wrap">
  <span class="sw-label off">AUS</span>
  <div class="flip" id="flipSw" onclick="doToggle()">
    <div class="flip-body">
      <div class="flip-track"></div>
      <div class="flip-knob %KNOBCLASS%" id="knob"></div>
    </div>
  </div>
  <span class="sw-label on">AN</span>
</div>
<div style="text-align:center"><div class="flip-ind %INDCLASS%" id="flipInd"></div><div style="font-size:.7em;color:#607d8b;margin-top:4px" id="flipLbl">%FLIPLBL%</div></div>

<h2>Sensorwerte <span style="font-size:.7em;color:#546e7a">(live)</span></h2>
<div class="vs">
  <div class="vc"><div class="l">Temperatur</div><div class="v" id="vt">--</div><div class="u">&deg;C</div></div>
  <div class="vc"><div class="l">Feuchte</div><div class="v" id="vh">--</div><div class="u">%</div></div>
  <div class="vc"><div class="l">CO&#8322;</div><div class="v" id="vc">--</div><div class="u">ppm</div></div>
  <div class="vc"><div class="l">VOC</div><div class="v" id="vv">--</div><div class="u">Index</div></div>
  <div class="vc"><div class="l">PM2.5</div><div class="v" id="vp">--</div><div class="u">&micro;g/m&sup3;</div></div>
</div>

<h2>Luftqualit&auml;t</h2>
<div id="aq" style="background:#1a1f2e;border-radius:10px;padding:10px;text-align:center;margin-bottom:6px">
  <span id="aqt" style="font-size:1.3em;font-weight:bold">--</span>
  <div id="aqd" style="font-size:.75em;color:#90a4ae;margin-top:2px"></div>
</div>

<h2>24h Verlauf &ndash; CO&#8322;</h2>
<div class="chart-wrap">
  <canvas id="chCo2" height="220"></canvas>
  <div class="legend">
    <span><span class="dot" style="background:#4caf50"></span> Gut &lt;800</span>
    <span><span class="dot" style="background:#ffeb3b"></span> M&auml;&szlig;ig</span>
    <span><span class="dot" style="background:#ff9800"></span> Schlecht</span>
    <span><span class="dot" style="background:#f44336"></span> Kritisch</span>
  </div>
</div>

<h2>24h Verlauf &ndash; VOC &amp; PM2.5</h2>
<div class="chart-wrap">
  <canvas id="chVP" height="180"></canvas>
  <div class="legend">
    <span><span class="dot" style="background:#7c4dff"></span> VOC Index</span>
    <span><span class="dot" style="background:#00e5ff"></span> PM2.5 &micro;g/m&sup3;</span>
  </div>
</div>

<div class="rn" id="ts">Aktualisierung...</div>

<script>
// Auto-Refresh Sensorwerte alle 5 Sekunden
function updVal(){
  fetch('/api/values').then(r=>r.json()).then(d=>{
    upd('vt',d.temp);upd('vh',d.hum);upd('vc',d.co2);
    upd('vv',d.voc);upd('vp',d.pm25);
    document.getElementById('ds').textContent=d.state;
    document.getElementById('scr').textContent=d.screen;
    // Dimm-Button aktualisieren
    var db=document.getElementById('dbtn');
    if(d.state=='Active'){
      db.textContent='\uD83C\uDF19 Dimmen';db.style.background='#ff9800';
    }else if(d.state=='Dimmed'){
      db.textContent='\u2600\uFE0F Helligkeit An';db.style.background='#4caf50';
    }else{
      db.textContent='\u2600\uFE0F Helligkeit An';db.style.background='#4caf50';
    }
    // Kippschalter aktualisieren
    var isOn=(d.state=='Active'||d.state=='Dimmed');
    var knob=document.getElementById('knob');
    var ind=document.getElementById('flipInd');
    var lbl=document.getElementById('flipLbl');
    knob.className='flip-knob '+(isOn?'on':'off');
    ind.className='flip-ind '+(isOn?'on':'off');
    lbl.textContent=isOn?'Display An':'Display Aus';
    document.getElementById('ts').textContent='Letztes Update: '+new Date().toLocaleTimeString();
    updAQ();
  }).catch(()=>{
    document.getElementById('upd').innerHTML='<span style="color:#f44336">&#9679; Offline</span>';
  });
}
function upd(id,val){
  var el=document.getElementById(id);
  if(el.textContent!=String(val)){
    el.textContent=val;el.classList.add('flash');
    setTimeout(()=>el.classList.remove('flash'),400);
  }
}
// Kippschalter Toggle
function doToggle(){
  fetch('/toggle').then(()=>setTimeout(updVal,300));
}
updVal();
setInterval(updVal,5000);

// Luftqualitaet Bewertung
function aqRating(co2,voc,pm){
  var r='Sehr gut',c='#4caf50';
  if(co2>1500||voc>250||pm>55){r='Kritisch';c='#f44336';}
  else if(co2>1000||voc>150||pm>35){r='Schlecht';c='#ff9800';}
  else if(co2>800||voc>100||pm>15){r='M\u00e4\u00dfig';c='#ffeb3b';}
  else if(co2>600||voc>60||pm>10){r='Gut';c='#8bc34a';}
  return {text:r,color:c};
}
function updAQ(){
  var co2=parseInt(document.getElementById('vc').textContent)||0;
  var voc=parseInt(document.getElementById('vv').textContent)||0;
  var pm=parseInt(document.getElementById('vp').textContent)||0;
  var q=aqRating(co2,voc,pm);
  var el=document.getElementById('aqt');
  el.textContent=q.text;el.style.color=q.color;
  var det=[];
  if(co2>0)det.push('CO\u2082: '+co2+' ppm'+(co2>1000?' \u26a0':''));
  if(voc>0)det.push('VOC: '+voc+(voc>150?' \u26a0':''));
  if(pm>0)det.push('PM2.5: '+pm+(pm>35?' \u26a0':''));
  document.getElementById('aqd').textContent=det.join(' | ');
}

// Chart Hilfsfunktionen
function drawZones(ctx,pad,cw,ch,yMax,zones){
  for(var i=0;i<zones.length;i++){
    var z=zones[i];
    var y1=pad.t+ch-ch*Math.min(z.to,yMax)/yMax;
    var y2=pad.t+ch-ch*z.from/yMax;
    ctx.fillStyle=z.bg;
    ctx.fillRect(pad.l,y1,cw,y2-y1);
    ctx.fillStyle=z.tc||'#607d8b';
    ctx.font='bold 18px sans-serif';ctx.textAlign='right';
    ctx.fillText(z.label,pad.l+cw-4,(y1+y2)/2+6);
  }
}
function drawGrid(ctx,pad,cw,ch,yMax,steps){
  ctx.strokeStyle='#2a2f3e44';ctx.lineWidth=1;
  ctx.font='20px sans-serif';ctx.fillStyle='#607d8b';ctx.textAlign='right';
  for(var i=0;i<=steps;i++){
    var y=pad.t+ch-ch*i/steps;
    ctx.beginPath();ctx.moveTo(pad.l,y);ctx.lineTo(pad.l+cw,y);ctx.stroke();
    ctx.fillText(Math.round(yMax*i/steps),pad.l-4,y+6);
  }
}
function drawTimeAxis(ctx,pad,cw,ch,times,n){
  ctx.fillStyle='#607d8b';ctx.font='18px sans-serif';ctx.textAlign='center';
  var step=Math.max(1,Math.floor(n/6));
  for(var i=0;i<n;i+=step){
    var x=pad.l+cw*i/(n-1);
    ctx.fillText(times[i],x,pad.t+ch+22);
  }
}
function drawLine(ctx,arr,n,maxVal,pad,cw,ch,color,lw){
  ctx.strokeStyle=color;ctx.lineWidth=lw||2.5;ctx.beginPath();
  for(var i=0;i<arr.length;i++){
    var x=pad.l+cw*i/(n-1);
    var y=pad.t+ch-ch*(Math.min(arr[i],maxVal)/maxVal);
    if(i===0)ctx.moveTo(x,y);else ctx.lineTo(x,y);
  }
  ctx.stroke();
}
function noData(cv,msg){
  var c=cv.getContext('2d'),W=cv.width,H=cv.height;
  c.fillStyle='#546e7a';c.font='22px sans-serif';c.textAlign='center';
  c.fillText(msg||'Noch keine Verlaufsdaten',W/2,H/2);
}

// CO2 Chart mit Qualitaetszonen
var cvCo2=document.getElementById('chCo2');
var ctxCo2=cvCo2.getContext('2d');
// VOC+PM Chart
var cvVP=document.getElementById('chVP');
var ctxVP=cvVP.getContext('2d');

function drawCharts(data){
  if(!data||!data.co2||data.co2.length<2){
    noData(cvCo2);noData(cvVP);return;
  }
  var n=data.co2.length;
  // === CO2 Chart ===
  var W1=cvCo2.width=cvCo2.offsetWidth*2;
  var H1=cvCo2.height=440;
  ctxCo2.clearRect(0,0,W1,H1);
  var p1={t:15,r:10,b:28,l:50};
  var cw1=W1-p1.l-p1.r, ch1=H1-p1.t-p1.b;
  var co2Max=Math.max(2000,...data.co2);
  co2Max=Math.max(co2Max,1600);co2Max=Math.ceil(co2Max/500)*500;
  // Qualitaetszonen
  drawZones(ctxCo2,p1,cw1,ch1,co2Max,[
    {from:0,to:800,bg:'rgba(76,175,80,0.15)',label:'Gut',tc:'#4caf5088'},
    {from:800,to:1000,bg:'rgba(255,235,59,0.12)',label:'M\u00e4\u00dfig',tc:'#ffeb3b88'},
    {from:1000,to:1500,bg:'rgba(255,152,0,0.12)',label:'Schlecht',tc:'#ff980088'},
    {from:1500,to:co2Max,bg:'rgba(244,67,54,0.12)',label:'Kritisch',tc:'#f4433688'}
  ]);
  drawGrid(ctxCo2,p1,cw1,ch1,co2Max,4);
  drawTimeAxis(ctxCo2,p1,cw1,ch1,data.times,n);
  drawLine(ctxCo2,data.co2,n,co2Max,p1,cw1,ch1,'#ff5252',3);
  // Aktuelller Wert Label
  var lastCo2=data.co2[n-1];
  var lx=p1.l+cw1,ly=p1.t+ch1-ch1*(Math.min(lastCo2,co2Max)/co2Max);
  ctxCo2.fillStyle='#ff5252';ctxCo2.font='bold 22px sans-serif';ctxCo2.textAlign='left';
  ctxCo2.fillText(lastCo2+'ppm',lx-70,ly-8);

  // === VOC+PM Chart ===
  var W2=cvVP.width=cvVP.offsetWidth*2;
  var H2=cvVP.height=360;
  ctxVP.clearRect(0,0,W2,H2);
  var p2={t:15,r:10,b:28,l:50};
  var cw2=W2-p2.l-p2.r, ch2=H2-p2.t-p2.b;
  var vpMax=Math.max(200,Math.max(...data.voc,...data.pm25));
  vpMax=Math.ceil(vpMax/50)*50;if(vpMax<100)vpMax=100;
  // VOC Zonen
  drawZones(ctxVP,p2,cw2,ch2,vpMax,[
    {from:0,to:100,bg:'rgba(76,175,80,0.10)',label:'',tc:'#4caf5066'},
    {from:100,to:200,bg:'rgba(255,235,59,0.08)',label:'',tc:'#ffeb3b66'},
    {from:200,to:vpMax,bg:'rgba(244,67,54,0.08)',label:'',tc:'#f4433666'}
  ]);
  drawGrid(ctxVP,p2,cw2,ch2,vpMax,4);
  drawTimeAxis(ctxVP,p2,cw2,ch2,data.times,n);
  drawLine(ctxVP,data.voc,n,vpMax,p2,cw2,ch2,'#7c4dff',2.5);
  drawLine(ctxVP,data.pm25,n,vpMax,p2,cw2,ch2,'#00e5ff',2.5);
  // Labels
  var lastVoc=data.voc[n-1],lastPm=data.pm25[n-1];
  ctxVP.font='bold 20px sans-serif';ctxVP.textAlign='left';
  ctxVP.fillStyle='#7c4dff';
  ctxVP.fillText('VOC:'+lastVoc,p2.l+8,p2.t+18);
  ctxVP.fillStyle='#00e5ff';
  ctxVP.fillText('PM:'+lastPm,p2.l+cw2/2,p2.t+18);
}
function loadChart(){
  fetch('/api/history').then(r=>r.json()).then(d=>drawCharts(d))
  .catch(()=>{noData(cvCo2);noData(cvVP);});
}
loadChart();
setInterval(loadChart,60000);
window.addEventListener('resize',loadChart);
</script>
</body>
</html>
)rawliteral";

// ============================================
// HELPER: Initiale Platzhalter ersetzen
// ============================================
static String buildPage() {
    String page = FPSTR(HTML_PAGE);
    
    int screenIdx = (int)ui_getCurrentScreen();
    const char* name = (screenIdx >= 0 && screenIdx < UI_SCREEN_COUNT) 
                       ? screenNames[screenIdx] : "?";
    page.replace("%SCREEN%", name);
    page.replace("%DISPSTATE%", String(powerManager.getStateString()));
    
    // Dimm-Button
    bool isActive = !powerManager.isDisplayOff() && !powerManager.isSleeping();
    if (!powerManager.isDimmed() && isActive) {
        page.replace("%DIMBTNTEXT%", "&#127769; Dimmen");
    } else {
        page.replace("%DIMBTNTEXT%", "&#9728;&#65039; Helligkeit An");
    }
    
    // Kippschalter Zustand
    bool dispOn = isActive;
    page.replace("%KNOBCLASS%", dispOn ? "on" : "off");
    page.replace("%INDCLASS%", dispOn ? "on" : "off");
    page.replace("%FLIPLBL%", dispOn ? "Display An" : "Display Aus");
    
    return page;
}

// ============================================
// ROUTE HANDLERS
// ============================================
static void handleRoot() {
    server.send(200, "text/html", buildPage());
}

static void handleNext() {
    powerManager.wakeUp();
    ui_nextScreen();
    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "OK");
}

static void handleScreen() {
    if (server.hasArg("id")) {
        int id = server.arg("id").toInt();
        if (id >= 0 && id < UI_SCREEN_COUNT) {
            powerManager.wakeUp();
            ui_setScreen((UIScreen)id);
        }
    }
    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "OK");
}

static void handleWake() {
    powerManager.wakeUp();
    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "OK");
}

static void handleDim() {
    if (powerManager.isDimmed()) {
        powerManager.wakeUp();
        Serial.println("[WEB] Dim: -> Active");
    } else {
        powerManager.dim();
        Serial.println("[WEB] Dim: -> Dimmed");
    }
    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "OK");
}

static void handleToggle() {
    if (powerManager.isDisplayOff() || powerManager.isSleeping()) {
        powerManager.wakeUp();
        Serial.println("[WEB] Toggle: -> Active");
    } else {
        powerManager.displayOff();
        Serial.println("[WEB] Toggle: -> Off");
    }
    server.send(200, "text/plain", "OK");
}

// ============================================
// JSON API: Aktuelle Sensorwerte
// ============================================
static void handleApiValues() {
    int screenIdx = (int)ui_getCurrentScreen();
    const char* scrName = (screenIdx >= 0 && screenIdx < UI_SCREEN_COUNT) 
                          ? screenNames[screenIdx] : "?";
    
    String json = "{";
    json += "\"temp\":" + String(sensorFilter.getSmoothedTemp(), 1) + ",";
    json += "\"hum\":" + String(sensorFilter.getSmoothedHum(), 0) + ",";
    json += "\"co2\":" + String(sensorFilter.getSmoothedCO2()) + ",";
    json += "\"voc\":" + String(sensorFilter.getSmoothedVOC()) + ",";
    json += "\"pm25\":" + String(sensorFilter.getSmoothedPM25()) + ",";
    json += "\"state\":\"" + String(powerManager.getStateString()) + "\",";
    json += "\"screen\":\"" + String(scrName) + "\"";
    json += "}";
    
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "application/json", json);
}

// ============================================
// JSON API: 24h Historie (Downsampled)
// ============================================
static void handleApiHistory() {
    int totalEntries = sensorHistory.getEntryCount();
    
    if (totalEntries < 2) {
        server.send(200, "application/json", "{\"co2\":[],\"voc\":[],\"pm25\":[],\"times\":[]}");
        return;
    }
    
    // Maximal 144 Punkte (alle 10 Minuten für 24h)
    int maxPoints = 144;
    int step = max(1, totalEntries / maxPoints);
    
    String json = "{\"co2\":[";
    String vocStr = "";
    String pm25Str = "";
    String timeStr = "";
    bool first = true;
    
    for (int i = 0; i < totalEntries; i += step) {
        HistoryEntry entry;
        if (!sensorHistory.getEntry(i, entry)) continue;
        
        if (!first) {
            json += ",";
            vocStr += ",";
            pm25Str += ",";
            timeStr += ",";
        }
        first = false;
        
        json += String(entry.co2);
        vocStr += String(entry.voc);
        pm25Str += String(entry.pm25);
        
        // Zeitformatierung
        if (entry.timestamp > 1600000000) {
            time_t t = entry.timestamp;
            struct tm* tm = localtime(&t);
            if (tm) {
                char buf[6];
                snprintf(buf, sizeof(buf), "%02d:%02d", tm->tm_hour, tm->tm_min);
                timeStr += "\"" + String(buf) + "\"";
            } else {
                timeStr += "\"--:--\"";
            }
        } else {
            // Relative Zeit (Minuten seit Boot)
            int mins = entry.timestamp / 60;
            int h = (mins / 60) % 24;
            int m = mins % 60;
            char buf[6];
            snprintf(buf, sizeof(buf), "%02d:%02d", h, m);
            timeStr += "\"" + String(buf) + "\"";
        }
    }
    
    json += "],\"voc\":[" + vocStr + "],\"pm25\":[" + pm25Str + "],\"times\":[" + timeStr + "]}";
    
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "application/json", json);
}

// ============================================
// PUBLIC API
// ============================================
void webRemote_begin() {
    server.on("/",              handleRoot);
    server.on("/next",          handleNext);
    server.on("/screen",        handleScreen);
    server.on("/wake",          handleWake);
    server.on("/dim",           handleDim);
    server.on("/toggle",        handleToggle);
    server.on("/api/values",    handleApiValues);
    server.on("/api/history",   handleApiHistory);
    server.begin();
    
    if (MDNS.begin("inspectair")) {
        MDNS.addService("http", "tcp", 80);
        Serial.println("[WEB] mDNS: http://inspectair.local");
    }
    
    Serial.println("[WEB] ═══════════════════════════════════════════");
    Serial.printf("[WEB]  Remote: http://%s\n", WiFi.localIP().toString().c_str());
    Serial.println("[WEB]  oder:  http://inspectair.local");
    Serial.println("[WEB]  API:   /api/values  /api/history");
    Serial.println("[WEB] ═══════════════════════════════════════════");
}

void webRemote_loop() {
    server.handleClient();
}
