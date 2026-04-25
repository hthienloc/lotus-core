// ==UserScript==
// @name         Jules Session & Message Copier (V3.3 Final)
// @namespace    http://tampermonkey.net/
// @version      3.3
// @description  Full bi-directional bridge with Regex ID extraction, Cache-Busting, and Strict Comment Preservation.
// @author       Gemini Orchestrator
// @match        https://jules.google.com/session/*
// @icon         https://www.google.com/s2/favicons?sz=64&domain=google.com
// @grant        GM_xmlhttpRequest
// @connect      localhost
// ==/UserScript==

/**
 * ARCHITECTURE OVERVIEW:
 * 
 * 1. SYNC (Jules -> Gemini): Transfers Jules message content to local relay.
 * 2. PROPOSAL (Gemini -> Jules): Polls for replies targeting this specific Session ID.
 * 3. APPROVAL: User confirms delivery of Gemini's reply to Jules via UI panel.
 * 4. SECURITY (CSP): Pure DOM construction (createElement) to bypass Google's strict CSP.
 * 5. CACHE BUSTING: Polling requests include a timestamp to ensure fresh data.
 * 6. URL RESILIENCE: Regex extraction ensuring correct Session ID in all sub-pages.
 * 7. STATE MANAGEMENT: Message fingerprinting prevents handled proposals from reappearing.
 */

(function() {
    'use strict';

    const RELAY_URL = 'http://localhost:8080';
    
    /**
     * Extracts the core Session ID from the URL using Regex.
     */
    function extractSessionID() {
        const match = window.location.href.match(/\/session\/([^\/]+)/);
        return match ? match[1] : null;
    }

    const SESSION_ID = extractSessionID();
    let handledMessages = new Set();

    console.log(`%c[Lotus Bridge V3.3] Active for Session: ${SESSION_ID}`, 'color: #3d5afe; font-weight: bold;');

    function getMessageFingerprint(msg) {
        try {
            return btoa(unescape(encodeURIComponent(msg))).slice(0, 32);
        } catch (e) {
            return msg.slice(0, 32);
        }
    }

    function getChatInput() {
        return document.querySelector('textarea') || document.querySelector('[contenteditable="true"]');
    }

    /**
     * Polling mechanism to receive instructions from Gemini.
     */
    function poll() {
        if (!SESSION_ID) return;
        const cacheBusterUrl = `${RELAY_URL}?t=${Date.now()}`;

        GM_xmlhttpRequest({
            method: "GET",
            url: cacheBusterUrl,
            onload: function(res) {
                try {
                    const data = JSON.parse(res.responseText);
                    
                    if (!data || !data.message || data.status !== 'pending') {
                        if (handledMessages.size > 0) {
                            handledMessages.clear();
                        }
                        const existing = document.getElementById('gemini-safe-panel');
                        if (existing) existing.remove();
                        return;
                    }

                    if (data.target_session_id === SESSION_ID) {
                        const fingerprint = getMessageFingerprint(data.message);
                        if (!handledMessages.has(fingerprint)) {
                            injectProposalUI(data.message, fingerprint);
                        }
                    }
                } catch (e) {}
            }
        });
    }

    /**
     * Displays the proposal panel.
     */
    function injectProposalUI(message, fingerprint) {
        if (document.getElementById('gemini-safe-panel')) return;

        const panel = document.createElement('div');
        panel.id = 'gemini-safe-panel';
        Object.assign(panel.style, {
            position: 'fixed', bottom: '120px', left: '50%', transform: 'translateX(-50%)',
            width: '450px', padding: '20px', background: '#ffffff', border: '4px solid #3d5afe',
            borderRadius: '16px', zIndex: '2147483647', boxShadow: '0 10px 40px rgba(0,0,0,0.4)',
            fontFamily: 'sans-serif'
        });

        const title = document.createElement('div');
        title.textContent = '⚡ GEMINI PROPOSED REPLY';
        Object.assign(title.style, { fontWeight: 'bold', color: '#1a237e', marginBottom: '12px', fontSize: '14px' });
        panel.appendChild(title);

        const msgBox = document.createElement('div');
        msgBox.textContent = message;
        Object.assign(msgBox.style, {
            fontSize: '13px', color: '#333', textAlign: 'left', maxHeight: '150px',
            overflowY: 'auto', background: '#f5f5f5', padding: '12px', borderRadius: '8px',
            border: '1px solid #ddd', whiteSpace: 'pre-wrap', marginBottom: '15px', lineHeight: '1.5'
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
                    const btn = document.querySelector('button[type="submit"]') || 
                                     document.querySelector('button[aria-label*="Send"]');
                    if (btn) btn.click();
                }, 100);
            }
            handledMessages.add(fingerprint);
            panel.remove();
            clearOutboxOnServer();
        };
        btnGroup.appendChild(sendBtn);

        const rejectBtn = document.createElement('button');
        rejectBtn.textContent = '❌ REJECT';
        Object.assign(rejectBtn.style, {
            flex: '1', padding: '12px', background: '#eceff1', color: '#37474f',
            border: 'none', borderRadius: '8px', cursor: 'pointer', fontWeight: 'bold'
        });
        rejectBtn.onclick = () => {
            handledMessages.add(fingerprint);
            panel.remove();
            clearOutboxOnServer();
        };
        btnGroup.appendChild(rejectBtn);

        panel.appendChild(btnGroup);
        document.body.appendChild(panel);
    }

    function clearOutboxOnServer() {
        GM_xmlhttpRequest({
            method: "POST",
            url: RELAY_URL,
            data: JSON.stringify({ action: 'clear_outbox' })
        });
    }

    function injectSyncButton(container, textGetter) {
        if (container.getAttribute('data-gemini-injected') === 'true') return;
        const btn = document.createElement('button');
        btn.textContent = '🚀 SYNC TO GEMINI';
        Object.assign(btn.style, {
            display: 'block', marginTop: '8px', padding: '6px 12px',
            background: '#1a73e8', color: 'white', border: 'none', borderRadius: '4px',
            fontSize: '11px', fontWeight: 'bold', cursor: 'pointer'
        });
        btn.onclick = () => {
            GM_xmlhttpRequest({
                method: "POST",
                url: RELAY_URL,
                data: JSON.stringify({ url: window.location.href, message: textGetter() }),
                headers: { "Content-Type": "application/json" }
            });
            btn.textContent = '✅ SYNCED';
            setTimeout(() => btn.textContent = '🚀 SYNC TO GEMINI', 2000);
        };
        container.appendChild(btn);
        container.setAttribute('data-gemini-injected', 'true');
    }

    function scanMessages() {
        document.querySelectorAll('swebot-agent-chat-bubble .message-container').forEach(container => {
            const content = container.querySelector('swebot-markdown-viewer') || container;
            injectSyncButton(container, () => content.innerText);
        });
    }

    setInterval(scanMessages, 1000);
    setInterval(poll, 3000);
})();
