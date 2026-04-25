// ==UserScript==
// @name         Jules Session & Message Copier (V2.7 Smart State)
// @namespace    http://tampermonkey.net/
// @version      2.7
// @description  Prevents duplicate popups using local message fingerprinting
// @author       Gemini Orchestrator
// @match        https://jules.google.com/session/*
// @icon         https://www.google.com/s2/favicons?sz=64&domain=google.com
// @grant        GM_xmlhttpRequest
// @connect      localhost
// ==/UserScript==

(function() {
    'use strict';

    const RELAY_URL = 'http://localhost:8080';
    const SESSION_ID = window.location.href.split('/').pop();
    
    // Ghi nhớ các tin nhắn đã xử lý để không hiện lại
    let handledMessages = new Set();

    console.log(`%c[Lotus Bridge V2.7] Smart State Active`, 'color: #7c4dff; font-weight: bold;');

    function getChatInput() {
        return document.querySelector('textarea') || document.querySelector('[contenteditable="true"]');
    }

    /**
     * Creates a unique ID for a message to track it
     */
    function getMessageFingerprint(msg) {
        return btoa(unescape(encodeURIComponent(msg))).slice(0, 32);
    }

    function poll() {
        GM_xmlhttpRequest({
            method: "GET",
            url: RELAY_URL,
            onload: function(res) {
                try {
                    const data = JSON.parse(res.responseText);
                    if (data.target_session_id === SESSION_ID && data.status === 'pending') {
                        const fingerprint = getMessageFingerprint(data.message);
                        
                        // CHỈ hiện nếu tin nhắn này chưa từng được xử lý
                        if (!handledMessages.has(fingerprint)) {
                            injectSafeUI(data.message, fingerprint);
                        }
                    } else {
                        // Nếu server báo không còn tin nhắn pending, xóa UI nếu đang hiện
                        const existing = document.getElementById('gemini-safe-panel');
                        if (existing && data.status !== 'pending') existing.remove();
                    }
                } catch (e) {}
            }
        });
    }

    function injectSafeUI(message, fingerprint) {
        if (document.getElementById('gemini-safe-panel')) return;

        const panel = document.createElement('div');
        panel.id = 'gemini-safe-panel';
        Object.assign(panel.style, {
            position: 'fixed', bottom: '120px', left: '50%', transform: 'translateX(-50%)',
            width: '450px', padding: '20px', background: '#ffffff', border: '4px solid #34a853',
            borderRadius: '16px', zIndex: '2147483647', boxShadow: '0 10px 40px rgba(0,0,0,0.4)',
            fontFamily: 'sans-serif'
        });

        const title = document.createElement('div');
        title.textContent = '🤖 GEMINI PROPOSED REPLY';
        Object.assign(title.style, { fontWeight: 'bold', color: '#1b5e20', marginBottom: '12px', fontSize: '14px' });
        panel.appendChild(title);

        const msgBox = document.createElement('div');
        msgBox.textContent = message;
        Object.assign(msgBox.style, {
            fontSize: '13px', color: '#333', textAlign: 'left', maxHeight: '150px',
            overflowY: 'auto', background: '#f1f8e9', padding: '12px', borderRadius: '8px',
            border: '1px solid #c5e1a5', whiteSpace: 'pre-wrap', marginBottom: '15px', lineHeight: '1.5'
        });
        panel.appendChild(msgBox);

        const btnGroup = document.createElement('div');
        Object.assign(btnGroup.style, { display: 'flex', gap: '10px' });

        const sendBtn = document.createElement('button');
        sendBtn.textContent = '✅ CONFIRM & SEND';
        Object.assign(sendBtn.style, {
            flex: '2', padding: '12px', background: '#2e7d32', color: 'white',
            border: 'none', borderRadius: '8px', cursor: 'pointer', fontWeight: 'bold'
        });
        
        sendBtn.onclick = () => {
            const chat = getChatInput();
            if (chat) {
                chat.value = message;
                chat.textContent = message;
                chat.dispatchEvent(new Event('input', { bubbles: true }));
                setTimeout(() => {
                    const btn = document.querySelector('button[type="submit"]') || document.querySelector('button[aria-label*="Send"]');
                    if (btn) btn.click();
                }, 100);
            }
            handledMessages.add(fingerprint); // Đánh dấu đã xử lý
            panel.remove();
            clearOutbox();
        };

        const rejectBtn = document.createElement('button');
        rejectBtn.textContent = '❌ REJECT';
        Object.assign(rejectBtn.style, {
            flex: '1', padding: '12px', background: '#eceff1', color: '#37474f',
            border: 'none', borderRadius: '8px', cursor: 'pointer', fontWeight: 'bold'
        });
        rejectBtn.onclick = () => {
            handledMessages.add(fingerprint); // Đánh dấu đã xử lý
            panel.remove();
            clearOutbox();
        };

        btnGroup.appendChild(sendBtn);
        btnGroup.appendChild(rejectBtn);
        panel.appendChild(btnGroup);
        document.body.appendChild(panel);
    }

    function clearOutbox() {
        GM_xmlhttpRequest({
            method: "POST",
            url: RELAY_URL,
            data: JSON.stringify({ action: 'clear_outbox' })
        });
    }

    function injectSync(c, getter) {
        if (c.getAttribute('data-gemini-injected')) return;
        const btn = document.createElement('button');
        btn.textContent = '🚀 SYNC TO GEMINI';
        Object.assign(btn.style, {
            display: 'block', marginTop: '8px', padding: '6px 12px',
            background: '#1a73e8', color: 'white', border: 'none', borderRadius: '4px',
            fontSize: '11px', fontWeight: 'bold', cursor: 'pointer'
        });
        btn.onclick = () => {
            GM_xmlhttpRequest({
                method: "POST", url: RELAY_URL,
                data: JSON.stringify({ url: window.location.href, message: getter() }),
                headers: { "Content-Type": "application/json" }
            });
            btn.textContent = '✅ SYNCED';
            setTimeout(() => btn.textContent = '🚀 SYNC TO GEMINI', 2000);
        };
        c.appendChild(btn);
        c.setAttribute('data-gemini-injected', 'true');
    }

    setInterval(() => {
        document.querySelectorAll('swebot-agent-chat-bubble .message-container').forEach(c => {
            const content = c.querySelector('swebot-markdown-viewer') || c;
            injectSync(c, () => content.innerText);
        });
    }, 1000);

    setInterval(poll, 3000);
})();
