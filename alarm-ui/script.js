
/*lcd configuration*/

const COLS = 16;
const CELL_W = 3.55;
const CELL_H = 5.95;
const DOT_W = 0.55;
const DOT_H = 0.65;
const DOT_SPACING_X = 0.6;
const DOT_SPACING_Y = 0.7;

const pixelGroup = document.getElementById("pixels");

let alarms = [];
let ringing = false;
let ringStart = 0;
let ringingAlarmId = null;
let blinkState = true;
let scrollOffset = 0;
let notificationActive = false;
let notificationText = "";
let notificationUser = "";
let notificationStart = 0;
let lastNotificationId = null;
let animationMode = false;
let animFrame = 0;
let animationType = "wave"; // "wave" | "dino"
let cactusX = 15;
let groundOffset = 0;

let jumping = false;
let jumpFrame = 0;

const days = ["SU","MO","TU","WE","TH","FR","SA"];

/*lcd font*/

const font = {

/*number*/
"0":["01110","10001","10011","10101","11001","10001","01110","00000"],
"1":["00100","01100","00100","00100","00100","00100","01110","00000"],
"2":["01110","10001","00001","00010","00100","01000","11111","00000"],
"3":["11110","00001","00001","01110","00001","00001","11110","00000"],
"4":["00010","00110","01010","10010","11111","00010","00010","00000"],
"5":["11111","10000","11110","00001","00001","10001","01110","00000"],
"6":["00110","01000","10000","11110","10001","10001","01110","00000"],
"7":["11111","00001","00010","00100","01000","01000","01000","00000"],
"8":["01110","10001","10001","01110","10001","10001","01110","00000"],
"9":["01110","10001","10001","01111","00001","00010","11100","00000"],

":":["00000","00100","00100","00000","00100","00100","00000","00000"],

" ":["00000","00000","00000","00000","00000","00000","00000","00000"],

/*abhabet characters*/
"A":["01110","10001","10001","11111","10001","10001","10001","00000"],
"B":["11110","10001","10001","11110","10001","10001","11110","00000"],
"C":["01110","10001","10000","10000","10000","10001","01110","00000"],
"D":["11110","10001","10001","10001","10001","10001","11110","00000"],
"E":["11111","10000","10000","11110","10000","10000","11111","00000"],
"F":["11111","10000","10000","11110","10000","10000","10000","00000"],
"G":["01110","10001","10000","10111","10001","10001","01110","00000"],
"H":["10001","10001","10001","11111","10001","10001","10001","00000"],
"I":["11111","00100","00100","00100","00100","00100","11111","00000"],
"J":["00111","00010","00010","00010","00010","10010","01100","00000"],
"K":["10001","10010","10100","11000","10100","10010","10001","00000"],
"L":["10000","10000","10000","10000","10000","10000","11111","00000"],
"M":["10001","11011","10101","10101","10001","10001","10001","00000"],
"N":["10001","11001","10101","10011","10001","10001","10001","00000"],
"O":["01110","10001","10001","10001","10001","10001","01110","00000"],
"P":["11110","10001","10001","11110","10000","10000","10000","00000"],
"Q":["01110","10001","10001","10001","10101","10010","01101","00000"],
"R":["11110","10001","10001","11110","10100","10010","10001","00000"],
"S":["01111","10000","10000","01110","00001","00001","11110","00000"],
"T":["11111","00100","00100","00100","00100","00100","00100","00000"],
"U":["10001","10001","10001","10001","10001","10001","01110","00000"],
"V":["10001","10001","10001","10001","10001","01010","00100","00000"],
"W":["10001","10001","10001","10101","10101","11011","10001","00000"],
"X":["10001","10001","01010","00100","01010","10001","10001","00000"],
"Y":["10001","10001","01010","00100","00100","00100","00100","00000"],
"Z":["11111","00001","00010","00100","01000","10000","11111","00000"],
"~":[
"00000",
"00100",
"01110",
"01110",
"01110",
"11111",
"00100",
"00000"
],
"@":[
"00110",
"01111",
"11110",
"11111",
"01110",
"01110",
"01010",
"00000"
],

"&":[
"00110",
"01111",
"11110",
"11111",
"01110",
"01110",
"00110",
"00000"
],

"#":[
"00100",
"00110",
"10101",
"10101",
"10101",
"00110",
"00100",
"00000"
],

".":[
"00000",
"00000",
"00000",
"00000",
"00000",
"00000",
"00100",
"00000"
],
"%":[
"00000",
"00000",
"00000",
"00100",
"01110",
"11111",
"00000",
"00000"
]

};

async function syncAnimation(mode){

  try{

    await fetch(
      "https://YOUR_FIREBASE_PROJECT.firebaseio.com/animation.json",
      {
        method:"PUT",
        headers:{ "Content-Type":"application/json" },
        body: JSON.stringify({ mode })
      }
    );

  }catch(e){
    console.error(e);
  }

}

/*lcd drawing */

function clearDisplay(){
  pixelGroup.innerHTML="";
}

function drawChar(char,col,row){
  const pattern = font[char] || font[" "];
  for(let y=0;y<8;y++){
    for(let x=0;x<5;x++){
      if(pattern[y][x]==="1"){
        const px = col*CELL_W + x*DOT_SPACING_X;
        const py = row*CELL_H + y*DOT_SPACING_Y;
        const rect=document.createElementNS("http://www.w3.org/2000/svg","rect");
        rect.setAttribute("x",px);
        rect.setAttribute("y",py);
        rect.setAttribute("width",DOT_W);
        rect.setAttribute("height",DOT_H);
        pixelGroup.appendChild(rect);
      }
    }
  }
}

function print(text,row){
  for(let i=0;i<text.length && i<COLS;i++){
    drawChar(text[i],i,row);
  }
}

function setLCD(l1,l2){
  clearDisplay();
  print(l1.padEnd(16," "),0);
  print(l2.padEnd(16," "),1);
}

// animation mode
function runWaveAnimation(){
  clearDisplay();
  for(let i=0;i<16;i++){
    let y = Math.floor((Math.sin((i+animFrame)/2)+1)*3);
    drawChar("%",i,y>3?1:0);
  }
  animFrame += 0.3;
}

function runDinoAnimation(){

  clearDisplay();

  /* scrolling ground */

  for(let i=0;i<16;i++){
    if((i + groundOffset) % 2 === 0){
      drawChar(".",i,1);
    }
  }

  groundOffset++;


  /* dino leg animation */

  const frame = Math.floor(animFrame % 2);
  const dinoChar = frame === 0 ? "@" : "&";


  /* jump logic */

  let dinoRow = jumping ? 0 : 1;

  drawChar(dinoChar,2,dinoRow);


  /* cactus movement */

  drawChar("#",cactusX,1);

  cactusX--;

  if(cactusX < -1){
    cactusX = 16;
  }


  /* collision reset (simple animation mode) */

  if(cactusX === 2 && !jumping){
    cactusX = 16;
  }


  /* jump animation timing */

  if(jumping){
    jumpFrame++;

    if(jumpFrame > 6){
      jumping = false;
      jumpFrame = 0;
    }
  }

  animFrame += 0.25;
}

function runAnimation(){
  if(animationType === "dino") runDinoAnimation();
  else runWaveAnimation();
}

function enableAnimationMode(){
  animationMode = true;
}

function enableDinoAnimationMode(){

  animationType = "dino";
  animationMode = true;
 syncAnimation("dino");
  cactusX = 15;
  groundOffset = 0;
  jumping = false;
  jumpFrame = 0;
  animFrame = 0;
}

function enableWaveAnimationMode(){

  animationType = "wave";
  animationMode = true;

  syncAnimation("wave");

}

function stopAnimationMode(){
  animationMode = false;
  syncAnimation("off");
  animFrame = 0;
  dinoX = 0;
  dinoStep = 0;
  cactusX = 15;
  cactusTick = 0;
  dinoSlideStep = 0;
}
/*clock screen*/

function toISODateLocal(dt){
  const y = dt.getFullYear();
  const m = String(dt.getMonth()+1).padStart(2,"0");
  const d = String(dt.getDate()).padStart(2,"0");
  return `${y}-${m}-${d}`;
}

function updateClock(){

  const now = new Date();
  let hour24 = now.getHours();
  const minute = now.getMinutes();
  const second = now.getSeconds();

  let displayHour = hour24;
  let period = "AM";

  if(hour24 >= 12) period="PM";
  if(hour24 == 0) displayHour=12;
  else if(hour24 > 12) displayHour-=12;

  const hh = String(displayHour).padStart(2,"0");
  const mm = String(minute).padStart(2,"0");
  const ss = String(second).padStart(2,"0");

  const dayName = days[now.getDay()];
  const topRow =
    (hh+":"+mm+":"+ss+" "+period)
    .padEnd(13," ")
    + " "+dayName;

    if(animationMode){

runAnimation();

if(notificationActive){
  animationMode = false;
}

return;
}

    /*slack notification*/

if(notificationActive){

  if(Date.now() - notificationStart > 8000){
    notificationActive = false;
    return;
  }

  let topRow = "SLACK MESSAGE".padEnd(16," ");

  let msg = notificationText;
  if(msg.length > 16){
    const padded = "   " + msg + "   ";
    const start = Math.floor(scrollOffset) % padded.length;
    msg = (padded + padded).substring(start,start+16);
    scrollOffset += 0.5;
  }else{
    msg = msg.padEnd(16," ");
  }

  setLCD(topRow,msg);
  return;
}

  /*ring mode*/

if (ringing) {

const alarm = alarms.find(a => a.id === ringingAlarmId);

if (Date.now() - ringStart >= 10000) {
  ringing = false;
  ringingAlarmId = null;
  scrollOffset = 0;
  return;
}


blinkState = !blinkState;


let topLeft = hh + ":" + mm + ":" + ss + " " + period;

let bellIcon = blinkState ? "~" : " ";

let displayTime =
  topLeft.padEnd(15," ") + bellIcon;


let noteText = (alarm?.name || "ALARM").toUpperCase();
let bottomRow;

if (noteText.length <= 16) {
  bottomRow = noteText.padEnd(16," ");
} else {
  const padded = "   " + noteText + "   ";
  const start = scrollOffset % padded.length;
  bottomRow = (padded + padded).substring(start, start + 16);

  scrollOffset += 0.5; 
}

setLCD(displayTime, bottomRow);
return;
}
  /*normal screen mode*/

  let nextAlarm=null;
  let bestDiff=Infinity;
  const nowMins = hour24*60 + minute;

  for(const a of alarms){

    if(!a.enabled) continue;

    if(a.scheduledDate){
      if(a.scheduledDate !== toISODateLocal(now)) continue;
    }

    const alarmMins = a.hour*60 + a.minute;
    const diff = (alarmMins - nowMins + 1440) % 1440;

    if(diff < bestDiff){
      bestDiff=diff;
      nextAlarm=a;
    }
  }

  let bottomRow="--:--";

  if(nextAlarm){

    let ah = nextAlarm.hour;
    let ap="AM";

    if(ah>=12) ap="PM";
    if(ah===0) ah=12;
    else if(ah>12) ah-=12;

    bottomRow =
      String(ah).padStart(2,"0")+":"+
      String(nextAlarm.minute).padStart(2,"0")+" "+
      ap;

     bottomRow = bottomRow.padEnd(15," ") + "~";
  }

  setLCD(topRow,bottomRow);

  /*trigger check*/
  if(nextAlarm && bestDiff===0 && second===0){

ringing=true;
ringStart=Date.now();
ringingAlarmId=nextAlarm.id;
animationMode=false;

}

  if(nextAlarm && bestDiff===0 && second===0){

    ringing=true;
    ringStart=Date.now();
    ringingAlarmId=nextAlarm.id;

    if(nextAlarm.scheduledDate === toISODateLocal(now)){
      nextAlarm.scheduledDate=null;
    }
  }
}

setInterval(updateClock,200);

/* set alarm /ui*/

const fabBtn = document.getElementById("fab-btn");
const timePickerOverlay = document.getElementById("time-picker-overlay");
const pickerHourInput = document.getElementById("picker-hour");
const pickerMinuteInput = document.getElementById("picker-minute");
const pickerCancel = document.getElementById("picker-cancel");
const pickerOk = document.getElementById("picker-ok");
const amChip = document.getElementById("ampm-am");
const pmChip = document.getElementById("ampm-pm");

const alarmList = document.getElementById("alarm-list");
const alarmCardTemplate = document.getElementById("alarm-card-template");

const detailsSheet = document.getElementById("details-sheet");
const detailsOverlay = document.getElementById("details-overlay");
const detailsSubtitle = document.getElementById("details-subtitle");
const detailsTime = document.getElementById("details-time");
const newAlarmCard = document.getElementById("new-alarm-card");
const detailsEdit = document.getElementById("details-edit");
const detailsDelete = document.getElementById("details-delete");
const detailsSave = document.getElementById("details-save");
const dayChips = document.querySelectorAll(".day-chip");
const detailsNameInput = document.getElementById("details-name-input");
const detailsDurationSlider = document.getElementById("details-duration-slider");
const detailsDurationLabel = document.getElementById("details-duration-label");
const scheduleAlarmBtn = document.getElementById("schedule-alarm-btn");
const scheduleAlarmIcon = document.getElementById("schedule-alarm-icon");
const scheduleAlarmText = document.getElementById("schedule-alarm-text");
const datePickerOverlay = document.getElementById("date-picker-overlay");
const dateCancel = document.getElementById("date-cancel");
const dateOk = document.getElementById("date-ok");
const selectedDateLabel = document.getElementById("selected-date-label");
const calMonthLabel = document.getElementById("cal-month-label");
const calGrid = document.getElementById("cal-grid");
const calPrev = document.getElementById("cal-prev");
const calNext = document.getElementById("cal-next");
const calClear = document.getElementById("cal-clear");
const calToday = document.getElementById("cal-today");

const navItems = document.querySelectorAll(".nav-item");
const screens = {
  alarms: document.getElementById("screen-alarms"),
  animations: document.getElementById("screen-animations"),
  connect: document.getElementById("screen-connect")
};

let pickerIsPm = false;

function openTimePicker(){
  timePickerOverlay.style.display = "flex";
}

if(detailsTime){
  detailsTime.style.cursor = "pointer";
  detailsTime.addEventListener("click",function(){
    if(!activeAlarmId) return;
    if(detailsSheet) detailsSheet.classList.remove("open");
    if(detailsOverlay) detailsOverlay.classList.remove("open");
    openTimePickerForEdit(activeAlarmId);
  });
}

function closeTimePicker(){
  timePickerOverlay.style.display = "none";
}

function setAm(isPm){
  pickerIsPm = isPm;
  if(isPm){
    pmChip.classList.add("active");
    amChip.classList.remove("active");
  }else{
    amChip.classList.add("active");
    pmChip.classList.remove("active");
  }
}

function refreshPickerDisplays(){
  const h = Math.min(12,Math.max(1,parseInt(pickerHourInput.value || "8",10)));
  const m = Math.min(59,Math.max(0,parseInt(pickerMinuteInput.value || "0",10)));
  pickerHourInput.value = String(h).padStart(2,"0");
  pickerMinuteInput.value = String(m).padStart(2,"0");
}

function sanitizeNumeric2(el){
  if(!el) return;
  el.value = String(el.value || "").replace(/\D/g,"").slice(0,2);
}

function attachTimeField(el){
  if(!el) return;
  el.addEventListener("input",function(){
    sanitizeNumeric2(el);
  });
  el.addEventListener("focus",function(){
    try{ el.select(); }catch(e){}
  });
  el.addEventListener("blur",function(){
    refreshPickerDisplays();
  });
}

attachTimeField(pickerHourInput);
attachTimeField(pickerMinuteInput);

function generateId(){
  return String(Date.now()) + "-" + Math.random().toString(16).slice(2);
}

function formatTime(hour24,minute){
  let displayHour = hour24;
  let period = "am";
  if(hour24 >= 12) period = "pm";
  if(hour24 === 0) displayHour = 12;
  else if(hour24 > 12) displayHour -= 12;
  return {
    hh: String(displayHour).padStart(2,"0"),
    mm: String(minute).padStart(2,"0"),
    period
  };
}

function normalizeAlarm(a){
  if(!a || typeof a !== "object") return null;
  if(typeof a.id !== "string") a.id = generateId();
  if(!Number.isFinite(a.hour)) a.hour = 8;
  if(!Number.isFinite(a.minute)) a.minute = 0;
  a.enabled = !!a.enabled;
  a.name = typeof a.name === "string" ? a.name : "Alarm";
  a.scheduledDate = typeof a.scheduledDate === "string" ? a.scheduledDate : null; // YYYY-MM-DD
  const rawDur = Number(a.durationSec);
  if(Number.isFinite(rawDur)){
    a.durationSec = Math.min(20,Math.max(5,Math.round(rawDur)));
  }else{
    a.durationSec = 10;
  }
  return a;
}

function loadAlarms(){
  try{
    const raw = localStorage.getItem("alarms");
    const parsed = raw ? JSON.parse(raw) : [];
    alarms = Array.isArray(parsed) ? parsed.map(normalizeAlarm).filter(Boolean) : [];
  }catch(e){
    alarms = [];
  }
}

async function syncAlarmsToFirebase(){
  try{
    await fetch(
      "https://YOUR_FIREBASE_PROJECT.firebaseio.com/alarms.json",
      {
        method:"PUT",
        headers:{ "Content-Type":"application/json" },
        body:JSON.stringify(alarms)
      }
    );
  }catch(e){
    // ignore
  }
}

function getNextEnabledAlarm(){
  const enabled = alarms.filter(a => a && a.enabled && Number.isFinite(a.hour) && Number.isFinite(a.minute));
  if(!enabled.length) return null;

  const now = new Date();
  const nowMins = now.getHours() * 60 + now.getMinutes();
  let best = null;
  let bestDiff = Infinity;

  for(const a of enabled){
    const alarmMins = a.hour * 60 + a.minute;
    const diff = (alarmMins - nowMins + 1440) % 1440;
    if(diff < bestDiff){
      bestDiff = diff;
      best = a;
    }
  }

  return best;
}

async function syncLegacyNextAlarmToFirebase(){
  const next = getNextEnabledAlarm();
  try{
    if(!next){
      await fetch(
        "https://YOUR_FIREBASE_PROJECT.firebaseio.com/alarm.json",
        { method:"DELETE" }
      );
      return;
    }

    await fetch(
      "https://YOUR_FIREBASE_PROJECT.firebaseio.com/alarm.json",
      {
        method:"PUT",
        headers:{ "Content-Type":"application/json" },
        body:JSON.stringify({ hour: next.hour, minute: next.minute })
      }
    );
  }catch(e){
    
  }
}

function persistAlarms(){
  try{
    localStorage.setItem("alarms",JSON.stringify(alarms));
  }catch(e){
    
  }
  syncAlarmsToFirebase();
  syncLegacyNextAlarmToFirebase();
}

async function fetchAlarmsOnce(){
  try{
    const res = await fetch(
      "https://YOUR_FIREBASE_PROJECT.firebaseio.com/alarms.json"
    );
    const data = await res.json();
    if(Array.isArray(data)){
      alarms = data.map(normalizeAlarm).filter(Boolean);
      persistAlarms();
      renderAlarms();
    }else{
      // backwards compatibility = old single-alarm schema at alarm.json
      try{
        const resLegacy = await fetch(
          "https://YOUR_FIREBASE_PROJECT.firebaseio.com/alarm.json"
        );
        const legacy = await resLegacy.json();
        if(legacy && legacy.hour != null){
          alarms = [normalizeAlarm({
            id: generateId(),
            hour: legacy.hour,
            minute: legacy.minute,
            enabled: true,
            name: "Alarm"
          })].filter(Boolean);
          persistAlarms();
          renderAlarms();
        }
      }catch(e2){
      
      }
    }
  }catch(e){
    
  }
}

let activeAlarmId = null;
let timePickerTargetId = null; // null = create

function getAlarmById(id){
  return alarms.find(a => a.id === id) || null;
}

function renderAlarms(){
  if(!alarmList || !alarmCardTemplate || !newAlarmCard) return;

  alarmList.querySelectorAll(".alarm-card[data-id]").forEach(el => el.remove());

  for(const alarm of alarms){
    const node = alarmCardTemplate.content.firstElementChild.cloneNode(true);
    node.dataset.id = alarm.id;

    const statusEl = node.querySelector(".alarm-caption-status");
    const labelEl = node.querySelector(".alarm-caption-label");
    const timeEl = node.querySelector(".alarm-time-display");
    const merEl = node.querySelector(".alarm-meridiem-display");
    const noteEl = node.querySelector(".alarm-note");
    const toggleEl = node.querySelector(".alarm-toggle");

    if(statusEl) statusEl.textContent = alarm.enabled ? "Scheduled" : "Off";
    if(labelEl) labelEl.textContent = alarm.name || "Alarm";

    const t = formatTime(alarm.hour,alarm.minute);
    if(timeEl) timeEl.textContent = t.hh + ":" + t.mm;
    if(merEl) merEl.textContent = t.period;
    if(noteEl) noteEl.textContent = "Tap for more info";

    if(toggleEl){
      toggleEl.classList.toggle("on",!!alarm.enabled);
      toggleEl.addEventListener("click",function(ev){
        ev.stopPropagation();
        alarm.enabled = !alarm.enabled;
        persistAlarms();
        renderAlarms();
        refreshDetails();
      });
    }

    node.addEventListener("click",function(){
      openDetails(alarm.id);
    });

    alarmList.insertBefore(node,newAlarmCard);
  }
}

function refreshDetails(){
  if(!activeAlarmId) return;
  const alarm = getAlarmById(activeAlarmId);
  if(!alarm){
    if(detailsSheet) detailsSheet.classList.remove("open");
    if(detailsOverlay) detailsOverlay.classList.remove("open");
    activeAlarmId = null;
    return;
  }

  const t = formatTime(alarm.hour,alarm.minute);
  if(detailsTime) detailsTime.textContent = t.hh + ":" + t.mm + " " + t.period.toUpperCase();
  if(detailsNameInput) detailsNameInput.value = alarm.name || "Alarm";

  if(detailsDurationSlider && detailsDurationLabel){
    const dur = Number.isFinite(alarm.durationSec) ? alarm.durationSec : 10;
    detailsDurationSlider.value = dur;
    detailsDurationLabel.textContent = dur + " sec";
  }

  const onOff = document.getElementById("details-onoff-text");
  if(onOff) onOff.textContent = alarm.enabled ? "Alarm is on" : "Alarm is off";

  updateScheduleButton();
}

function openDetails(alarmId){
  activeAlarmId = alarmId;
  refreshDetails();
  if(detailsSheet) detailsSheet.classList.add("open");
  if(detailsOverlay) detailsOverlay.classList.add("open");
}

function openTimePickerForNew(){
  timePickerTargetId = null;
  pickerHourInput.value = 8;
  pickerMinuteInput.value = 0;
  setAm(false);
  refreshPickerDisplays();
  openTimePicker();
}

function openTimePickerForEdit(alarmId){
  const alarm = getAlarmById(alarmId);
  if(!alarm) return;
  timePickerTargetId = alarmId;

  pickerIsPm = alarm.hour >= 12;
  let displayHour = alarm.hour;
  if(displayHour === 0) displayHour = 12;
  else if(displayHour > 12) displayHour -= 12;

  pickerHourInput.value = displayHour;
  pickerMinuteInput.value = alarm.minute;
  setAm(pickerIsPm);
  refreshPickerDisplays();
  openTimePicker();
}

fabBtn.addEventListener("click",openTimePickerForNew);
newAlarmCard.addEventListener("click",openTimePickerForNew);
pickerCancel.addEventListener("click",closeTimePicker);

pickerOk.addEventListener("click",async function(){
  refreshPickerDisplays();
  let hour = parseInt(pickerHourInput.value || "8",10);
  const minute = parseInt(pickerMinuteInput.value || "0",10);

  if(hour === 12) hour = 0;
  if(pickerIsPm) hour += 12;

  if(timePickerTargetId){
    const alarm = getAlarmById(timePickerTargetId);
    if(alarm){
      alarm.hour = hour;
      alarm.minute = minute;
      alarm.enabled = true;
    }
  }else{
    alarms.push(normalizeAlarm({
      id: generateId(),
      hour,
      minute,
      enabled: true,
      name: "Alarm",
      scheduledDate: null,
      durationSec: 10
    }));
  }

  persistAlarms();
  renderAlarms();
  refreshDetails();
  closeTimePicker();
});

amChip.addEventListener("click",function(){ setAm(false); });
pmChip.addEventListener("click",function(){ setAm(true); });

dayChips.forEach(function(chip){
  chip.addEventListener("click",function(){
    chip.classList.toggle("active");
  });
});

let selectedDate = null; // date edited in modal
let viewYear = (new Date()).getFullYear();
let viewMonth = (new Date()).getMonth();

function updateScheduleButton(){
  if(!scheduleAlarmBtn || !scheduleAlarmText || !scheduleAlarmIcon) return;
  const alarm = activeAlarmId ? getAlarmById(activeAlarmId) : null;
  const isScheduled = !!(alarm && alarm.scheduledDate);
  scheduleAlarmBtn.disabled = !alarm;
  scheduleAlarmBtn.style.opacity = alarm ? "1" : "0.4";
  scheduleAlarmBtn.style.pointerEvents = alarm ? "auto" : "none";
  if(isScheduled){
    scheduleAlarmText.textContent = "Cancel";
    scheduleAlarmIcon.textContent = "cancel";
  }else{
    scheduleAlarmText.textContent = "Schedule alarm";
    scheduleAlarmIcon.textContent = "calendar_today";
  }
}

function formatMonthYear(year,monthIndex){
  const dt = new Date(year,monthIndex,1);
  return dt.toLocaleString(undefined,{ month:"long", year:"numeric" });
}

function formatSelectedDate(dt){
  return dt.toLocaleDateString(undefined,{ year:"numeric", month:"long", day:"numeric" });
}

function toISODateLocal(dt){
  const y = dt.getFullYear();
  const m = String(dt.getMonth()+1).padStart(2,"0");
  const d = String(dt.getDate()).padStart(2,"0");
  return `${y}-${m}-${d}`;
}

function parseISODateLocal(iso){
  const m = /^(\d{4})-(\d{2})-(\d{2})$/.exec(String(iso || ""));
  if(!m) return null;
  const y = parseInt(m[1],10);
  const mo = parseInt(m[2],10) - 1;
  const d = parseInt(m[3],10);
  return new Date(y,mo,d);
}

function isSameDay(a,b){
  return a && b &&
    a.getFullYear() === b.getFullYear() &&
    a.getMonth() === b.getMonth() &&
    a.getDate() === b.getDate();
}

function renderCalendar(){
  if(!calGrid || !calMonthLabel) return;

  calMonthLabel.textContent = formatMonthYear(viewYear,viewMonth);
  if(selectedDateLabel){
    selectedDateLabel.textContent = selectedDate ? formatSelectedDate(selectedDate) : "Selected date";
  }

  calGrid.innerHTML = "";
  const firstDay = new Date(viewYear,viewMonth,1).getDay(); // 0=Sun
  const daysInMonth = new Date(viewYear,viewMonth+1,0).getDate();
  const daysInPrevMonth = new Date(viewYear,viewMonth,0).getDate();

  // 6 rows * 7 cols = 42 cells
  for(let cell=0; cell<42; cell++){
    const dayNum = cell - firstDay + 1;
    let cellYear = viewYear;
    let cellMonth = viewMonth;
    let dateNum = dayNum;
    let isOut = false;

    if(dayNum < 1){
      isOut = true;
      dateNum = daysInPrevMonth + dayNum;
      cellMonth = viewMonth - 1;
      if(cellMonth < 0){ cellMonth = 11; cellYear = viewYear - 1; }
    }else if(dayNum > daysInMonth){
      isOut = true;
      dateNum = dayNum - daysInMonth;
      cellMonth = viewMonth + 1;
      if(cellMonth > 11){ cellMonth = 0; cellYear = viewYear + 1; }
    }

    const btn = document.createElement("button");
    btn.type = "button";
    btn.className = "cal-day" + (isOut ? " out" : "");
    btn.textContent = String(dateNum);

    const dt = new Date(cellYear,cellMonth,dateNum);
    if(selectedDate && isSameDay(selectedDate,dt)){
      btn.classList.add("selected");
    }

    btn.addEventListener("click",function(){
      selectedDate = dt;
      viewYear = dt.getFullYear();
      viewMonth = dt.getMonth();
      renderCalendar();
    });

    calGrid.appendChild(btn);
  }
}

function openDatePicker(){
  const alarm = activeAlarmId ? getAlarmById(activeAlarmId) : null;
  if(!alarm) return;
  if(datePickerOverlay){
    datePickerOverlay.style.display = "flex";
  }
  selectedDate = alarm.scheduledDate ? parseISODateLocal(alarm.scheduledDate) : (selectedDate || new Date());
  const base = selectedDate || new Date();
  viewYear = base.getFullYear();
  viewMonth = base.getMonth();
  renderCalendar();
}

if(scheduleAlarmBtn){
  scheduleAlarmBtn.addEventListener("click",function(){
    const alarm = activeAlarmId ? getAlarmById(activeAlarmId) : null;
    if(!alarm) return;
    if(alarm.scheduledDate){
      alarm.scheduledDate = null;
      persistAlarms();
      updateScheduleButton();
      return;
    }
    openDatePicker();
  });
}

function closeDatePicker(){
  if(datePickerOverlay){
    datePickerOverlay.style.display = "none";
  }
}

if(dateCancel) dateCancel.addEventListener("click",closeDatePicker);
if(dateOk) dateOk.addEventListener("click",function(){
  const alarm = activeAlarmId ? getAlarmById(activeAlarmId) : null;
  if(alarm){
    alarm.scheduledDate = selectedDate ? toISODateLocal(selectedDate) : null;
    persistAlarms();
    updateScheduleButton();
  }
  closeDatePicker();
});
if(datePickerOverlay){
  datePickerOverlay.addEventListener("click",function(e){
    if(e.target === datePickerOverlay) closeDatePicker();
  });
}

if(calPrev){
  calPrev.addEventListener("click",function(){
    viewMonth -= 1;
    if(viewMonth < 0){ viewMonth = 11; viewYear -= 1; }
    renderCalendar();
  });
}

if(calNext){
  calNext.addEventListener("click",function(){
    viewMonth += 1;
    if(viewMonth > 11){ viewMonth = 0; viewYear += 1; }
    renderCalendar();
  });
}

if(calToday){
  calToday.addEventListener("click",function(){
    selectedDate = new Date();
    viewYear = selectedDate.getFullYear();
    viewMonth = selectedDate.getMonth();
    renderCalendar();
  });
}

if(calClear){
  calClear.addEventListener("click",function(){
    selectedDate = null;
    renderCalendar();
  });
}

updateScheduleButton();

detailsEdit.addEventListener("click",function(){
  if(!activeAlarmId) return;
  if(detailsSheet) detailsSheet.classList.remove("open");
  if(detailsOverlay) detailsOverlay.classList.remove("open");
  openTimePickerForEdit(activeAlarmId);
});

detailsDelete.addEventListener("click",function(){
  if(!activeAlarmId) return;
  alarms = alarms.filter(a => a.id !== activeAlarmId);
  persistAlarms();
  renderAlarms();
  if(detailsSheet) detailsSheet.classList.remove("open");
  if(detailsOverlay) detailsOverlay.classList.remove("open");
  activeAlarmId = null;
});

detailsSave.addEventListener("click",function(){
  if(activeAlarmId){
    const alarm = getAlarmById(activeAlarmId);
    if(alarm){
      if(detailsNameInput){
        alarm.name = String(detailsNameInput.value || "Alarm").trim() || "Alarm";
      }
      if(detailsDurationSlider){
        const dur = parseInt(detailsDurationSlider.value || "10",10);
        alarm.durationSec = Math.min(20,Math.max(5,isNaN(dur)?10:dur));
      }
      persistAlarms();
      renderAlarms();
      refreshDetails();
    }
  }
  if(detailsSheet) detailsSheet.classList.remove("open");
  if(detailsOverlay) detailsOverlay.classList.remove("open");
});

if(detailsOverlay){
  detailsOverlay.addEventListener("click",function(){
    if(detailsSheet) detailsSheet.classList.remove("open");
    detailsOverlay.classList.remove("open");
  });
}

if(detailsDurationSlider && detailsDurationLabel){
  detailsDurationSlider.addEventListener("input",function(){
    const v = parseInt(detailsDurationSlider.value || "10",10);
    const clamped = Math.min(20,Math.max(5,isNaN(v)?10:v));
    detailsDurationLabel.textContent = clamped + " sec";
  });
}

function showScreen(key){
  Object.keys(screens).forEach(function(name){
    var el = screens[name];
    if(!el) return;
    el.style.display = (name === key) ? "flex" : "none";
  });
  navItems.forEach(function(item){
    item.classList.toggle("active", item.dataset.screen === key);
  });
}

navItems.forEach(function(item){
  item.addEventListener("click",function(){
    var target = item.dataset.screen || "alarms";
    showScreen(target);
  });
});

showScreen("alarms");

const startAnimationBtn = document.getElementById("start-animation");
if(startAnimationBtn){
  startAnimationBtn.addEventListener("click", enableWaveAnimationMode);
}

const startDinoBtn = document.getElementById("start-dino-animation");
if(startDinoBtn){
  startDinoBtn.addEventListener("click", enableDinoAnimationMode);
}

document.querySelectorAll("#stop-animation").forEach(btn=>{
  btn.addEventListener("click", stopAnimationMode);
});

refreshPickerDisplays();
loadAlarms();
renderAlarms();

function listenSlackNotifications(){

  const url = "https://YOUR_FIREBASE_PROJECT.firebaseio.com/slack_messages.json";

  setInterval(async ()=>{

    if(notificationActive) return;

    try{

      const res = await fetch(url);
      const data = await res.json();

      if(!data) return;

      const keys = Object.keys(data);
      const latestKey = keys[keys.length - 1];

      /*ignore if already shown notification*/
      if(latestKey === lastNotificationId) return;

      const last = data[latestKey];
      if(!last || !last.text) return;

      lastNotificationId = latestKey;

      notificationText = String(last.text).toUpperCase();
      notificationUser = String(last.user || "SLACK");

      notificationActive = true;
      notificationStart = Date.now();

    }catch(e){
      console.error(e);
    }

  },3000);

}

listenSlackNotifications();


/* Slack connect */

const slackBtn = document.getElementById("slack-connect-btn");

function updateSlackUI(){

  const connected = localStorage.getItem("slack_connected");

  if(connected && slackBtn){
    slackBtn.textContent = "Connected";
    slackBtn.style.background = "#000";
    slackBtn.style.color = "#fff";
  }

}

if(slackBtn){

  slackBtn.addEventListener("click", function(){

    localStorage.setItem("slack_connected","true");

    window.open(
      "https://hackclub.slack.com/oauth?client_id=2210535565.10679650721600&scope=users%3Aread%2Capp_mentions%3Aread%2Cchannels%3Ahistory%2Cim%3Ahistory%2Cim%3Aread&user_scope=&redirect_uri=&state=&granular_bot_scope=1&single_channel=0&install_redirect=install-on-team&tracked=1&user_default=0&team=1",
      "_blank"
    );

    updateSlackUI();

  });

}

updateSlackUI();

function startWave(){

  enableWaveAnimationMode();

  const card = document.querySelectorAll(".animation-card")[0];

  const play = card.querySelector(".play-btn");
  const stop = card.querySelector(".stop-btn");

  play.classList.add("active");
  stop.classList.remove("disabled");

}

function stopWave(){

  stopAnimationMode();

  const card = document.querySelectorAll(".animation-card")[0];

  const play = card.querySelector(".play-btn");
  const stop = card.querySelector(".stop-btn");

  play.classList.remove("active");
  stop.classList.add("disabled");

}


function startDino(){

  enableDinoAnimationMode();

  const card = document.querySelectorAll(".animation-card")[1];

  const play = card.querySelector(".play-btn");
  const stop = card.querySelector(".stop-btn");

  play.classList.add("active");
  stop.classList.remove("disabled");

}

function stopDino(){

  stopAnimationMode();

  const card = document.querySelectorAll(".animation-card")[1];

  const play = card.querySelector(".play-btn");
  const stop = card.querySelector(".stop-btn");

  play.classList.remove("active");
  stop.classList.add("disabled");

}