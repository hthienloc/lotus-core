// ==UserScript==
// @name         Jules Session & Message Copier (Direct Link Edition)
// @namespace    http://tampermonkey.net/
// @version      1.9
// @description  Automated bridge with Local Relay Server
// @author       Gemini Orchestrator
// @match        https://jules.google.com/session/*
// @icon         https://www.google.com/s2/favicons?sz=64&domain=google.com
// @grant        GM_xmlhttpRequest
// @connect      localhost
// ==/UserScript==

(function() {
    'use strict';

    const RELAY_URL = 'http://localhost:8080';

    console.log('%c[Lotus Bridge] Version 1.9 Active (Direct Link)', 'color: #34a853; font-weight: bold;');

    function sendToRelay(data) {
        fetch(RELAY_URL, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: json.stringify(data)
        })
        .then(response => console.log('[Lotus Bridge] Message delivered to local relay.'))
        .catch(error => console.error('[Lotus Bridge] Relay error:', error));
    }

    function createCopyButton(container, textGetter) {
        if (container.getAttribute('data-gemini-injected') === 'true') return;
        
        const btn = document.createElement('button');
        btn.textContent = '🚀 SYNC TO GEMINI';
        Object.assign(btn.style, {
            display: 'block', marginTop: '10px', padding: '8px 16px',
            background: '#4285f4', color: 'white', border: 'none', borderRadius: '8px',
            fontSize: '12px', fontWeight: 'bold', cursor: 'pointer',
            boxShadow: '0 2px 4px rgba(0,0,0,0.2)'
        });

        btn.onclick = () => {
            const payload = {
                url: window.location.href,
                message: textGetter(),
                timestamp: new Date().toISOString()
            };

            // 1. Copy to clipboard (as fallback)
            const textArea = document.createElement("textarea");
            textArea.value = `Session URL: ${payload.url}\n\nMessage:\n${payload.message}`;
            document.body.appendChild(textArea);
            textArea.select();
            document.execCommand('copy');
            document.body.removeChild(textArea);

            // 2. Send to Local Relay
            fetch(RELAY_URL, {
                method: 'POST',
                mode: 'no-cors', // To avoid preflight issues in some browsers
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(payload)
            });

            btn.textContent = '⚡ SYNCED!';
            btn.style.background = '#34a853';
            setTimeout(() => {
                btn.textContent = '🚀 SYNC TO GEMINI';
                btn.style.background = '#4285f4';
            }, 2000);
        };

        container.appendChild(btn);
        container.setAttribute('data-gemini-injected', 'true');
    }

    function scan() {
        const containers = document.querySelectorAll('swebot-agent-chat-bubble .message-container');
        containers.forEach(container => {
            const content = container.querySelector('swebot-markdown-viewer') || container;
            createCopyButton(container, () => content.innerText);
        });
    }

    setInterval(scan, 1000);
})();
