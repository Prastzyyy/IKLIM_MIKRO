//web.cpp

#include "web.h"

String html;

String SendHTML(float temperature, float humidity, int lightValue, String kondisi_kipas, String kondisi_sprayer, String kondisi_lampu){ 
html.reserve(5000);
html = "<!DOCTYPE html> <html>\n";
html += "   <head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
html += "   <title>Dashboard</title>\n";
html += "       <style>html { font-family: Helvetica, Arial, sans-serif; display: inline-block; margin: 0px auto; text-align: center; background-color: var(--color1);transition: background-color 0.3s ease, color 0.3s ease;}\n";
html += "           .light-theme {--color1 : #f4f4f9;--color2 : #444444;--color3 : #666666;--color4 : #444444;--color5 : #ffffff;--color6 : #dddddd;--color7 : #ffffff;--color8 : #dddddd;--color9 : #4CAF50;--color10 : white;--color11 : #45a049;--color12 : #ffffff;--color13 : #dddddd;--color14 : #f0f0f0;}\n";
html += "           .dark-theme {--color1 : #1a1a1a;--color2 : #8a2be2;--color3 : #8a2be2;--color4 : #dcdcdc;--color5 : #555555;--color6 : #dcdcdc;--color7 : #555555;--color8 : #dcdcdc;--color9 : #8a2be2;--color10 : #dcdcdc;--color11 : #7a1db3;--color12 : #8a2be2;--color13 : #dcdcdc;--color14 : #7a1db3;}\n";
html += "           body { margin-top: 50px; color: #333; }\n";
html += "           body.light-theme {background-color: var(--color1);}\n";
html += "           body.dark-theme {background-color: var(--color1);}\n";
html += "           h1 { color: var(--color2); margin: 50px auto 30px; font-size: 2rem; }\n";
html += "           h2 { color: var(--color3); margin: 30px auto 20px; font-size: 1.5rem; }\n";
html += "           p { font-size: 1.2rem; color: var(--color4); margin-bottom: 20px; }\n";

html += "           .sensor-grid { display: grid; grid-template-columns: repeat(3, 1fr); gap: 10px; max-width: 500px; margin: 20px auto; }\n";
html += "           .sensor-box { background-color: var(--color5); border: 1px solid var(--color6); border-radius: 10px; padding: 20px; box-shadow: 0px 2px 4px rgba(0, 0, 0, 0.1); }\n";

html += "           .status-grid { display: grid; grid-template-columns: repeat(3, 1fr); gap: 10px; max-width: 500px; margin: 20px auto; }\n";
html += "           .status-box { background-color: var(--color5); border: 1px solid var(--color6); border-radius: 10px; padding: 20px; box-shadow: 0px 2px 4px rgba(0, 0, 0, 0.1); }\n";

html += "           .kontrol-grid { display: grid; grid-template-columns: repeat(3, 1fr); gap: 10px; max-width: 500px; margin: 20px auto; }\n";
html += "           .kontrol-box { background-color: var(--color5); border: 1px solid var(--color6); border-radius: 10px; padding: 20px; box-shadow: 0px 2px 4px rgba(0, 0, 0, 0.1); }\n";
html += "           .kontrol-box button {background-color: var(--color9);border: none;color: var(--color10);padding: 5px 5px;font-size: 1rem;cursor: pointer;border-radius: 5px;box-shadow: 0px 2px 4px rgba(0, 0, 0, 0.1);transition: background-color 0.3s ease;}\n";
html += "           .kontrol-box button:hover {background-color: var(--color11);}\n";

html += "           .theme-switch {position: fixed;top: 10px;right: 10px;padding: 5px 5px;background-color: var(--color12);opacity: 0.3;border: 1px solid var(--color13);border-radius: 5px;cursor: pointer;font-size: 1rem;color: var(--color4);transition: background-color 0.3s ease;}\n";
html += "           .theme-switch:hover {background-color: var(--color14);}\n";
html += "       </style>\n";
html += "   </head>\n";
html += "   <body class=\"light-theme\">\n";
html += "       <h1>Sistem Monitoring Iklim Mikro</h1>\n";

html += "       <div id=\"sensor\">\n";
html += "       <h2>Pembacaan Sensor</h2>\n";
html += "           <div class=\"sensor-grid\">\n";
html += "               <div class=\"sensor-box\">\n";
html += "                   <p><strong>Suhu</strong> : <br><span id=\"Suhu\">";
html +=                     temperature;
html += "                   </span> <sup>o</sup>C</p>\n";
html += "               </div>\n";
html += "               <div class=\"sensor-box\">\n";
html += "                   <p><strong>Kelembaban</strong>: <br><span id=\"Kelembaban\">";
html +=                     humidity;
html += "                   </span> %</p>\n";
html += "               </div>\n";
html += "               <div class=\"sensor-box\">\n";
html += "                   <p><strong>Cahaya</strong>: <br><span id=\"Cahaya\">";
html +=                     lightValue;
html += "                   </span> </p>\n";
html += "               </div>\n";
html += "           </div>\n";
html += "       </div>\n";

html += "       <div id=\"aktuator\">\n";
html += "       <h2>Kondisi Aktuator</h2>\n";
html += "           <div class=\"status-grid\">\n";
html += "               <div class=\"status-box\">\n";
html += "                   <p><strong>Kipas</strong> : <br><span id=\"Kipas\">";
html +=                     kondisi_kipas;
html += "                   </span> </p>\n";
html += "               </div>\n";
html += "               <div class=\"status-box\">\n";
html += "                   <p><strong>Sprayer</strong>: <br><span id=\"Sprayer\">";
html +=                     kondisi_sprayer;
html += "                   </span></p>\n";
html += "               </div>\n";
html += "               <div class=\"status-box\">\n";
html += "                   <p><strong>Lampu</strong>:<br> <span id=\"Lampu\">";
html +=                     kondisi_lampu;
html += "                   </span> </p>\n";
html += "               </div>\n";
html += "           </div>\n";
html += "       </div>\n";

html += "       <div id=\"kontrol\">\n";
html += "       <h2>Kontrol Sistem</h2>\n";
html += "           <div class=\"kontrol-grid\">\n";
html += "               <div class=\"kontrol-box\">\n";
html += "                   <p><strong>Kipas</strong> : <br>\n";
html += "                       <button onclick=\"controlKipas('/kipas_on')\">ON</button>\n";
html += "                       <button onclick=\"controlKipas('/kipas_off')\">OFF</button>\n";
html += "               </div>\n";

html += "               <div class=\"kontrol-box\">\n";
html += "                   <p><strong>Sprayer</strong> : <br>\n";
html += "                       <button onclick=\"controlSprayer('/sprayer_on')\">ON</button>\n";
html += "                       <button onclick=\"controlSprayer('/sprayer_off')\">OFF</button>\n";
html += "               </div>\n";
html += "               <div class=\"kontrol-box\">\n";
html += "                   <p><strong>Lampu</strong>: <br>\n";
html += "                       <button onclick=\"controlLampu('/lampu_on')\">ON</button>\n";
html += "                       <button onclick=\"controlLampu('/lampu_off')\">OFF</button>\n";
html += "               </div>\n";
html += "           </div>\n";
html += "       </div>\n";

html += "       <div class=\"theme-switch\" onclick=\"toggleTheme()\">Ganti Tema</div>\n";

html += "       <script>\n";
html += "       function controlKipas(endpoint) {\n";
html += "         fetch(endpoint)\n";
html += "           .then(response => response.text())\n";
html += "           .then(data => {\n";
html += "             console.log(data);\n";
html += "           });\n";
html += "       }\n";

html += "       function controlSprayer(endpoint) {\n";
html += "         fetch(endpoint)\n";
html += "           .then(response => response.text())\n";
html += "           .then(data => {\n";
html += "             console.log(data);\n";
html += "           });\n";
html += "       }\n";

html += "       function controlLampu(endpoint) {\n";
html += "         fetch(endpoint)\n";
html += "           .then(response => response.text())\n";
html += "           .then(data => {\n";
html += "             console.log(data);\n";
html += "           });\n";
html += "       }\n";

html += "       function toggleTheme() {\n";
html += "         const body = document.body;\n";
html += "         const currentTheme = body.classList.contains('light-theme') ? 'light-theme' : 'dark-theme';\n";
html += "         body.classList.toggle('light-theme', currentTheme === 'dark-theme');\n";
html += "         body.classList.toggle('dark-theme', currentTheme === 'light-theme');\n";
html += "         const buttonText = body.classList.contains('dark-theme') ? 'Tema Terang' : 'Tema Gelap';\n";
html += "         document.querySelector('.theme-switch').textContent = buttonText;\n";
html += "       }\n";

html += "       setInterval(() => {\n";
html += "         fetch('/data')\n";
html += "           .then(res => res.json())\n";
html += "           .then(data => {\n";
html += "             document.getElementById('Suhu').textContent = data.temperature;\n";
html += "             document.getElementById('Kelembaban').textContent = data.humidity;\n";
html += "             document.getElementById('Cahaya').textContent = data.lightValue;\n";
html += "             document.getElementById('Kipas').textContent = data.kondisi_kipas;\n";
html += "             document.getElementById('Sprayer').textContent = data.kondisi_sprayer;\n";
html += "             document.getElementById('Lampu').textContent = data.kondisi_lampu;\n";
html += "           });\n";
html += "       }, 2000);\n";
html += "     </script>\n";
html += "   </body>\n";
html += "</html>\n";

return html; 
}
