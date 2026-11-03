#pragma once
#include <Arduino.h>

const char kWebPage[] PROGMEM = R"HTML(
<!doctype html><html lang="vi"><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>TH03 · Nhà kính</title>
<style>
*{box-sizing:border-box}body{margin:0;background:#eef3f2;color:#183c35;font:16px system-ui,sans-serif}
main{max-width:1050px;margin:auto;padding:32px 20px}h1{font-size:32px;margin:10px 0}small{color:#53736b}
.notice{padding:16px;background:#fff1ca;border-radius:12px;margin:22px 0}.sensors,.cards{display:grid;gap:16px;grid-template-columns:repeat(3,1fr)}
article,.sensors div{background:white;border:1px solid #d8e3df;border-radius:16px;padding:22px}.value{font-size:28px;display:block;margin-top:8px}
.cards{margin:24px 0}h2{margin:0 0 16px}.state{font-weight:bold}.muted{color:#637a74}button,select{font:inherit;padding:11px;border-radius:8px;border:1px solid #b4c9c2}
button{cursor:pointer;background:#166953;color:white}button.stop{background:#9d3838}button:disabled{opacity:.4;cursor:not-allowed}.actions{display:flex;gap:10px;margin-top:20px}select{width:100%;margin-top:8px}#message{min-height:26px}progress{width:100%;accent-color:#16866b}
@media(max-width:700px){.cards,.sensors{grid-template-columns:1fr}}
</style><main><small>CSE3069 / TH03</small><h1>Điều khiển nhà kính</h1>
<div class="notice"><strong>LED1 · LED2 · MOTOR1 — ĐIỀU KHIỂN THẬT</strong><br>LED dùng PWM, HIGH bật / LOW tắt. Trạng thái hiển thị là lệnh xuất, không phải phản hồi đo từ tải. MOTOR2 ngoài phạm vi.</div>
<p id="connection" role="status">Đang kết nối…</p>
<section class="sensors"><div>Nhiệt độ <b class="value" id="temperature">—</b></div><div>Độ ẩm <b class="value" id="humidity">—</b></div><div>Dữ liệu UNO <b id="sensor" class="value">—</b></div></section>
<p class="muted">Giải mã cảm biến theo giả định đã chọn. Chỉ cho chạy logic khi có mẫu hợp lệ; dữ liệu quá 6 giây được coi là lỗi.</p>
<section class="cards" id="cards"></section><button class="stop" id="stopAll">DỪNG MOTOR1 VÀ HAI KÊNH LED</button>
<p id="message" role="status"></p><small>Mỗi kênh tự dừng sau 20 giây. Đóng trang không làm mất bộ đếm trên ESP32.</small></main>
<script>
const names=['LED1','LED2','MOTOR1'];let snapshot=null,online=false,busy=false;
const reasons={BOOT:'Khởi động',START:'Lệnh bật',STOP:'Lệnh tắt',TIMEOUT:'Hết 20 giây',SENSOR_ERROR:'Dữ liệu UNO lỗi / quá cũ',PWM_ERROR:'PWM error',INVALID_COMMAND:'Lệnh không hợp lệ'};
document.querySelector('#cards').innerHTML=names.map((n,i)=>`<article><h2>${n}</h2><p class="state" id="state${i}">CHƯA XÁC NHẬN</p><p id="reason${i}">—</p><progress id="progress${i}" max="20" value="0"></progress><p id="time${i}">—</p>${i<2?`<label>Mức PWM dự kiến<select id="duty${i}" aria-label="PWM ${n}"><option>25</option><option selected>50</option><option>90</option></select></label>`:`<label>Pump PWM (%)<select id="duty${i}" aria-label="Pump PWM"><option>25</option><option>50</option><option>75</option><option selected>100</option></select></label><p>Duty %, not measured flow.</p>`}<div class="actions"><button id="start${i}">BẬT</button><button class="stop" id="stop${i}">TẮT</button></div></article>`).join('');
function controls(){names.forEach((_,i)=>{document.querySelector('#start'+i).disabled=!online||busy||!snapshot?.sensorValid||!snapshot?.channels[i]?.hardwareEnabled;document.querySelector('#stop'+i).disabled=!online;document.querySelector('#duty'+i).disabled=!online||busy;});document.querySelector('#stopAll').disabled=!online;}
async function request(url,options={}){const r=await fetch(url,{...options,signal:AbortSignal.timeout(3500),cache:'no-store'});const data=await r.json();if(!r.ok)throw new Error(data.error||'Lệnh bị từ chối');return data;}
function lost(){online=false;document.querySelector('#connection').textContent='Mất kết nối — trạng thái chưa xác nhận';names.forEach((_,i)=>{document.querySelector('#state'+i).textContent='CHƯA XÁC NHẬN';document.querySelector('#time'+i).textContent='—';});controls();}
function render(s){snapshot=s;online=true;document.querySelector('#connection').textContent='ESP32: LED1 / LED2 / MOTOR1 hardware';document.querySelector('#temperature').textContent=s.sensorValid?s.temperature.toFixed(1)+' °C':'—';document.querySelector('#humidity').textContent=s.sensorValid?s.humidity.toFixed(1)+' %':'—';document.querySelector('#sensor').textContent=s.sensorValid?'HỢP LỆ':'CHƯA HỢP LỆ';s.channels.forEach((c,i)=>{document.querySelector('#state'+i).textContent=({STOPPED:'DỪNG',RUNNING:c.hardwareEnabled?'RUNNING (OUTPUT ON)':'OUTPUT ERROR',FAULT:'LỖI'})[c.state];document.querySelector('#reason'+i).textContent=reasons[c.reason]||c.reason;document.querySelector('#time'+i).textContent='Còn '+Math.ceil(c.remainingMs/1000)+' giây';document.querySelector('#progress'+i).value=c.remainingMs/1000;document.querySelector('#duty'+i).value=c.duty;});controls();}
async function command(channel,action,duty){if(action!=='STOP'&&busy)return;const revision=snapshot?.channels[channel]?.revision;busy=true;controls();try{const body=new URLSearchParams({channel,action});if(revision!==undefined)body.set('revision',revision);if(duty!==undefined)body.set('duty',duty);await request('/api/control',{method:'POST',body});document.querySelector('#message').textContent='Command processed by ESP32.';}catch(e){document.querySelector('#message').textContent=e.message;}finally{busy=false;try{render(await request('/api/status'));}catch{lost();}controls();}}
names.forEach((_,i)=>{document.querySelector('#start'+i).onclick=()=>command(i,'START');document.querySelector('#stop'+i).onclick=()=>command(i,'STOP');document.querySelector('#duty'+i).onchange=e=>command(i,'PWM',e.target.value);});document.querySelector('#stopAll').onclick=()=>command('all','STOP');
async function poll(){try{render(await request('/api/status'));}catch{lost();}setTimeout(poll,1000);}controls();poll();
</script></html>
)HTML";
