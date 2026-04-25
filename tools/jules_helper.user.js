// ==UserScript==
// @name         Jules Session & Message Copier
// @namespace    http://tampermonkey.net/
// @version      1.1
// @description  Copy Jules message and session URL for debugging
// @author       Gemini Orchestrator
// @match        https://jules.google.com/session/*
// @icon         https://www.google.com/s2/favicons?sz=64&domain=google.com
// @grant        none
// ==/UserScript==

(function() {
    'use strict';

    /**
     * Copies text to the system clipboard.
     * @param {string} text
     */
    function copyToClipboard(text) {
        navigator.clipboard.writeText(text).then(() => {
            console.log('Copied to clipboard!');
        }).catch(err => {
            console.error('Failed to copy: ', err);
        });
    }

    /**
     * Injects a copy button into a message container.
     * @param {HTMLElement} container
     * @param {Function} textGetter
     */
    function createCopyButton(container, textGetter) {
        if (container.querySelector('.gemini-copy-btn')) return;

        const btn = document.createElement('button');
        btn.innerHTML = '📋 Copy Info';
        btn.className = 'gemini-copy-btn';
        btn.style.cssText = `
            position: absolute;
            top: 5px;
            right: 5px;
            padding: 4px 8px;
            background: #4285f4;
            color: white;
            border: none;
            border-radius: 4px;
            cursor: pointer;
            font-size: 10px;
            z-index: 1000;
            opacity: 0.7;
            transition: opacity 0.2s;
        `;
        
        btn.onmouseover = () => btn.style.opacity = '1';
        btn.onmouseout = () => btn.style.opacity = '0.7';

        btn.onclick = (e) => {
            e.preventDefault();
            e.stopPropagation();
            const message = textGetter();
            const sessionUrl = window.location.href;
            const fullText = `Session URL: ${sessionUrl}\n\nMessage:\n${message}`;
            copyToClipboard(fullText);
            
            const originalText = btn.innerHTML;
            btn.innerHTML = '✅ Copied!';
            setTimeout(() => { btn.innerHTML = originalText; }, 2000);
        };

        container.style.position = 'relative';
        container.appendChild(btn);
    }

    // Observer to handle dynamic content loading in Jules UI
    const observer = new MutationObserver((mutations) => {
        // Target Agent (Jules) chat bubbles
        const agentBubbles = document.querySelectorAll('swebot-agent-chat-bubble .message-container');
        agentBubbles.forEach(bubble => {
            createCopyButton(bubble, () => {
                const mdViewer = bubble.querySelector('swebot-markdown-viewer');
                return mdViewer ? mdViewer.innerText : 'No text found';
            });
        });

        // Target User chat bubbles
        const userBubbles = document.querySelectorAll('swebot-user-chat-bubble .message-container');
        userBubbles.forEach(bubble => {
            createCopyButton(bubble, () => {
                const mdViewer = bubble.querySelector('swebot-markdown-viewer');
                return mdViewer ? mdViewer.innerText : 'No text found';
            });
        });
    });

    observer.observe(document.body, {
        childList: true,
        subtree: true
    });
})();
