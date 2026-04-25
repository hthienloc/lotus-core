// ==UserScript==
// @name         Jules Session & Message Copier (V3.1 Production)
// @namespace    http://tampermonkey.net/
// @version      3.1
// @description  A robust, bi-directional bridge with cache-busting, CSP compliance, and strict state management.
// @author       Gemini Orchestrator
// @match        https://jules.google.com/session/*
// @icon         https://www.google.com/s2/favicons?sz=64&domain=google.com
// @grant        GM_xmlhttpRequest
// @connect      localhost
// ==/UserScript==

/**
 * ARCHITECTURE OVERVIEW:
 * 
 * 1. SYNC (Jules -> Gemini): User clicks "SYNC TO GEMINI" on a Jules message. 
 *    The script sends the message content and session URL to a local Relay Server.
 * 
 * 2. PROPOSAL (Gemini -> Jules): The script polls the local Relay Server for any pending 
 *    replies from Gemini meant for this specific Session ID.
 * 
 * 3. APPROVAL (User Intervention): If a reply is found, a high-visibility UI panel 
 *    is shown. The user must click "CONFIRM & SEND" to actually deliver the message 
 *    to Jules, maintaining human-in-the-loop safety.
 * 
 * 4. SECURITY (CSP/Trusted Types): The UI is built using surgical DOM methods (createElement) 
 *    to bypass Google's strict Content Security Policy which blocks innerHTML.
 * 
 * 5. CACHE BUSTING: Polling requests include a timestamp parameter to prevent browser 
 *    caching of local relay responses.
 */

(function() {
    'use strict';

    // Local Relay Server Configuration
    const RELAY_URL = 'http://localhost:8080';
    const SESSION_ID = window.location.href.split('/').pop();
    
    // In-memory set to prevent showing the same proposal multiple times in one session
    let handledMessages = new Set();

    console.log(`%c[Lotus Bridge V3.1] Production Cache-Buster Active for: ${SESSION_ID}`, 'color: #3d5afe; font-weight: bold;');

    /**
     * Generates a simple fingerprint to uniquely identify a message string.
     * Used to track which proposals have already been handled.
     */
    function getMessageFingerprint(msg) {
        try {
            return btoa(unescape(encodeURIComponent(msg))).slice(0, 32);
        } catch (e) {
            return msg.slice(0, 32);
        }
    }

    /**
     * Attempts to locate the chat input area in Jules' UI.
     * Supports both standard textareas and contenteditable divs.
     */
    function getChatInput() {
        return document.querySelector('textarea') || document.querySelector('[contenteditable="true"]');
    }

    /**
     * Periodically polls the local relay server for messages from Gemini.
     * Uses cache-busting timestamp to ensure fresh data.
     */
    function poll() {
        const cacheBusterUrl = `${RELAY_URL}?t=${Date.now()}`;

        GM_xmlhttpRequest({
            method: "GET",
            url: cacheBusterUrl,
            timeout: 2000,
            onload: function(res) {
                try {
                    const data = JSON.parse(res.responseText);
                    
                    // Logic Cleanup: If server outbox is empty, reset local memory
                    if (!data || !data.message || data.status !== 'pending') {
                        if (handledMessages.size > 0) {
                            console.log('[Lotus Bridge] Server cleared. Resetting local state.');
                            handledMessages.clear();
                        }
                        const existing = document.getElementById('gemini-safe-panel');
                        if (existing) existing.remove();
                        return;
                    }

                    // Check if this proposal matches our specific Jules session
                    if (data.target_session_id === SESSION_ID) {
                        const fingerprint = getMessageFingerprint(data.message);
                        if (!handledMessages.has(fingerprint)) {
                            injectProposalUI(data.message, fingerprint);
                        }
                    }
                } catch (e) {
                    // Fail silently for network issues/relay down
                }
            }
        });
    }

    /**
     * Builds and displays a safe, CSP-compliant UI for Gemini's proposed reply.
     */
    function injectProposalUI(message, fingerprint) {
        if (document.getElementById('gemini-safe-panel')) return;

        // Container Panel
        const panel = document.createElement('div');
        panel.id = 'gemini-safe-panel';
        Object.assign(panel.style, {
            position: 'fixed', bottom: '120px', left: '50%', transform: 'translateX(-50%)',
            width: '450px', padding: '20px', background: '#ffffff', border: '4px solid #3d5afe',
            borderRadius: '16px', zIndex: '2147483647', boxShadow: '0 10px 40px rgba(61,90,254,0.3)',
            fontFamily: 'sans-serif'
        });

        // Header
        const title = document.createElement('div');
        title.textContent = '⚡ GEMINI PROPOSED REPLY';
        Object.assign(title.style, { fontWeight: 'bold', color: '#1a237e', marginBottom: '12px', fontSize: '14px' });
        panel.appendChild(title);

        // Message Content Area
        const msgBox = document.createElement('div');
        msgBox.textContent = message;
        Object.assign(msgBox.style, {
            fontSize: '13px', color: '#333', textAlign: 'left', maxHeight: '150px',
            overflowY: 'auto', background: '#f5f5f5', padding: '12px', borderRadius: '8px',
            border: '1px solid #ddd', whiteSpace: 'pre-wrap', marginBottom: '15px', lineHeight: '1.5'
        });
        panel.appendChild(msgBox);

        // Buttons
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
                // Simulate human input to trigger Jules/Angular listeners
                chat.value = message;
                chat.textContent = message;
                chat.dispatchEvent(new Event('input', { bubbles: true }));
                setTimeout(() => {
                    const sendBtnReal = document.querySelector('button[type="submit"]') || 
                                     document.querySelector('button[aria-label*="Send"]');
                    if (sendBtnReal) sendBtnReal.click();
                }, 100);
            }
            handledMessages.add(fingerprint);
            panel.remove();
            clearOutboxOnServer();
        };

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

        btnGroup.appendChild(sendBtn);
        btnGroup.appendChild(rejectBtn);
        panel.appendChild(btnGroup);
        document.body.appendChild(panel);
    }

    /**
     * Commands the local relay server to empty the outbox.
     */
    function clearOutboxOnServer() {
        GM_xmlhttpRequest({
            method: "POST",
            url: RELAY_URL,
            data: JSON.stringify({ action: 'clear_outbox' })
        });
    }

    /**
     * Injects the "SYNC TO GEMINI" button into individual Jules message bubbles.
     */
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

    /**
     * Scans the DOM for message bubbles to inject sync buttons.
     */
    function scanMessages() {
        document.querySelectorAll('swebot-agent-chat-bubble .message-container').forEach(container => {
            const content = container.querySelector('swebot-markdown-viewer') || container;
            injectSyncButton(container, () => content.innerText);
        });
    }

    // High-frequency scan for sync buttons
    setInterval(scanMessages, 1000);
    
    // Lower-frequency poll for Gemini replies
    setInterval(poll, 3000);
})();
