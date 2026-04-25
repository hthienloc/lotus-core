// ==UserScript==
// @name         Jules Session & Message Copier (V2.0 Bi-directional)
// @namespace    http://tampermonkey.net/
// @version      2.0
// @description  Full bi-directional bridge with Session ID filtering
// @author       Gemini Orchestrator
// @match        https://jules.google.com/session/*
// @icon         https://www.google.com/s2/favicons?sz=64&domain=google.com
// @grant        none
// ==/UserScript==

(function() {
    'use strict';

    const RELAY_URL = 'http://localhost:8080';
    const SESSION_ID = window.location.href.split('/').pop();

    console.log(`%c[Lotus Bridge V2] Active for Session: ${SESSION_ID}`, 'color: #4285f4; font-weight: bold; font-size: 14px;');

    function getChatInput() {
        return document.querySelector('textarea') || document.querySelector('[contenteditable="true"]');
    }

    /**
     * Polling Gemini for replies meant for THIS session
     */
    async function pollForReplies() {
        try {
            const res = await fetch(RELAY_URL);
            const data = await res.json();

            if (data.target_session_id === SESSION_ID && data.status === 'pending') {
                console.log('[Lotus Bridge] New reply received from Gemini!');
                injectApprovalUI(data.message);
            }
        } catch (e) {
            // Relay might be down, ignore
        }
    }

    function injectApprovalUI(message) {
        if (document.getElementById('gemini-approval-panel')) return;

        const panel = document.createElement('div');
        panel.id = 'gemini-approval-panel';
        Object.assign(panel.style, {
            position: 'fixed', top: '20px', left: '50%', transform: 'translateX(-50%)',
            width: '400px', padding: '15px', background: '#fff9c4', border: '3px solid #fbc02d',
            borderRadius: '12px', zIndex: '200000', boxShadow: '0 8px 30px rgba(0,0,0,0.3)',
            fontFamily: 'sans-serif', textAlign: 'center'
        });

        panel.innerHTML = `
            <div style="font-weight:bold; color:#f57f17; margin-bottom:10px;">🤖 GEMINI PROPOSED A REPLY</div>
            <div style="font-size:12px; background:white; padding:10px; border-radius:4px; text-align:left; max-height:150px; overflow-y:auto; border:1px solid #ddd;">
                ${message.replace(/\n/g, '<br>')}
            </div>
            <div style="margin-top:15px; display:flex; gap:10px; justify-content:center;">
                <button id="gemini-approve-btn" style="padding:8px 20px; background:#2e7d32; color:white; border:none; border-radius:6px; cursor:pointer; font-weight:bold;">✅ CONFIRM & SEND</button>
                <button id="gemini-reject-btn" style="padding:8px 20px; background:#c62828; color:white; border:none; border-radius:6px; cursor:pointer; font-weight:bold;">❌ REJECT</button>
            </div>
        `;

        document.body.appendChild(panel);

        document.getElementById('gemini-approve-btn').onclick = () => {
            const chat = getChatInput();
            if (chat) {
                chat.value = message;
                chat.textContent = message;
                chat.dispatchEvent(new Event('input', { bubbles: true }));
                
                const sendBtn = document.querySelector('button[type="submit"]') || 
                                 document.querySelector('button[aria-label*="Send"]');
                if (sendBtn) sendBtn.click();
            }
            panel.remove();
            markReplyAsDone();
        };

        document.getElementById('gemini-reject-btn').onclick = () => {
            panel.remove();
            markReplyAsDone();
        };
    }

    async function markReplyAsDone() {
        // Tell local relay to clear/mark as done
        await fetch(RELAY_URL, {
            method: 'POST',
            mode: 'no-cors',
            body: JSON.stringify({ action: 'clear_outbox' })
        });
    }

    // Reuse the SYNC button from V1.9
    function createSyncButton(container, textGetter) {
        if (container.getAttribute('data-gemini-injected') === 'true') return;
        const btn = document.createElement('button');
        btn.textContent = '🚀 SYNC TO GEMINI';
        Object.assign(btn.style, {
            display: 'block', marginTop: '10px', padding: '6px 12px',
            background: '#1a73e8', color: 'white', border: 'none', borderRadius: '4px',
            fontSize: '11px', fontWeight: 'bold', cursor: 'pointer'
        });
        btn.onclick = async () => {
            const payload = { url: window.location.href, message: textGetter() };
            await fetch(RELAY_URL, { method: 'POST', mode: 'no-cors', body: JSON.stringify(payload) });
            btn.textContent = '⚡ SYNCED!';
            setTimeout(() => btn.textContent = '🚀 SYNC TO GEMINI', 2000);
        };
        container.appendChild(btn);
        container.setAttribute('data-gemini-injected', 'true');
    }

    function scan() {
        document.querySelectorAll('swebot-agent-chat-bubble .message-container').forEach(c => {
            const content = c.querySelector('swebot-markdown-viewer') || c;
            createSyncButton(c, () => content.innerText);
        });
    }

    setInterval(scan, 1000);
    setInterval(pollForReplies, 3000); // Poll for replies every 3 seconds
})();
